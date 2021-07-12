#include "FGSDKInternal.h"

#include "GSDKConfiguration.h"
#include "GSDKUtils.h"
#include "HttpModule.h"
#include "Async/Async.h"
#include "Json.h"
#include "PlayfabGSDK.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"
#include "HAL/Event.h"

FGSDKInternal::FGSDKInternal()
	: TransitionToActiveEvent(FPlatformProcess::GetSynchEventFromPool(false))
	, SignalHeartbeatEvent(FPlatformProcess::GetSynchEventFromPool(false))
{
	TUniquePtr<FConfiguration> ConfigPtr;

	FString FileName = UGSDKUtils::GetEnvironmentVariable(TEXT("GSDK_CONFIG_FILE"));

	// If the configuration file is not there, we'll get our config from environment variables
	if (!FPaths::FileExists(FileName))
	{
		ConfigPtr = MakeUnique<FEnvironmentVariableConfiguration>();
	}
	else
	{
		ConfigPtr = MakeUnique<FJsonFileConfiguration>(FileName);
	}

	TMap<FString, FString> GameCerts = ConfigPtr->GetGameCertificates();
	for (const auto& GameCert: GameCerts)
	{
		ConfigSettings.Add(GameCert);
	}

	TMap<FString, FString> MetaDatas = ConfigPtr->GetBuildMetadata();
	for (const auto& MetaData: MetaDatas)
	{
		ConfigSettings.Add(MetaData);
	}

	TMap<FString, FString> Ports = ConfigPtr->GetGamePorts();
	for (const auto& Port: Ports)
	{
		ConfigSettings.Add(Port);
	}

	ConfigSettings.Add(FPlayfabGSDKModule::HEARTBEAT_ENDPOINT_KEY, ConfigPtr->GetHeartbeatEndpoint());
	ConfigSettings.Add(FPlayfabGSDKModule::SERVER_ID_KEY, ConfigPtr->GetServerId());
	ConfigSettings.Add(FPlayfabGSDKModule::LOG_FOLDER_KEY, ConfigPtr->GetLogFolder());
	ConfigSettings.Add(FPlayfabGSDKModule::SHARED_CONTENT_FOLDER_KEY, ConfigPtr->GetSharedContentFolder());
	ConfigSettings.Add(FPlayfabGSDKModule::CERTIFICATE_FOLDER_KEY, ConfigPtr->GetCertificateFolder());
	ConfigSettings.Add(FPlayfabGSDKModule::TITLE_ID_KEY, ConfigPtr->GetTitleId());
	ConfigSettings.Add(FPlayfabGSDKModule::BUILD_ID_KEY, ConfigPtr->GetBuildId());
	ConfigSettings.Add(FPlayfabGSDKModule::REGION_KEY, ConfigPtr->GetRegion());
	ConfigSettings.Add(FPlayfabGSDKModule::PUBLIC_IP_V4_ADDRESS_KEY, ConfigPtr->GetPublicIpV4Address());
	ConfigSettings.Add(FPlayfabGSDKModule::FULLY_QUALIFIED_DOMAIN_NAME_KEY, ConfigPtr->GetFullyQualifiedDomainName());

	if (ConfigSettings[FPlayfabGSDKModule::HEARTBEAT_ENDPOINT_KEY].IsEmpty() || ConfigSettings[FPlayfabGSDKModule::SERVER_ID_KEY].IsEmpty())
	{
		UE_LOG(LogPlayfabGSDK, Fatal, TEXT("Heartbeat endpoint and Server id are required configuration values."));
		return;
	}

	ConnectionInfo = ConfigPtr->GetGameServerConnectionInfo();

	if (ConfigPtr->ShouldLog())
	{
		StartLog();
	}

	// Use highest frequency permitted heartbeat interval until VMAgent tells an updated one.
	NextHeartbeatIntervalMs = 1000; // TODO Move to constant

	FString GsmBaseUrl = ConfigSettings[FPlayfabGSDKModule::HEARTBEAT_ENDPOINT_KEY];
	FString InstanceId = ConfigSettings[FPlayfabGSDKModule::SERVER_ID_KEY];

	UE_LOG(LogPlayfabGSDK, Log, TEXT("VM Agent Endpoint: %s"), *GsmBaseUrl);
	UE_LOG(LogPlayfabGSDK, Log, TEXT("Instance Id: %s"), *InstanceId);

	HeartbeatUrl = FString::Printf(TEXT("http://%s/v1/sessionHosts/%s"), *GsmBaseUrl, *InstanceId);

	CachedScheduledMaintenance = FDateTime(0);

	HttpHeaders.Add(TEXT("Accept"), TEXT("application/json"));
	HttpHeaders.Add(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));

	TransitionToActiveEvent->Reset();
	SignalHeartbeatEvent->Reset();

	KeepHeartbeatRunning = ConfigPtr->ShouldHeartbeat();

	HeartbeatThread = Async(EAsyncExecution::Thread,
	[this]()
        {
            HeartbeatAsyncTaskFunction();
        }
	);
}

FGSDKInternal::~FGSDKInternal()
{
	KeepHeartbeatRunning = false;
	FPlatformProcess::ReturnSynchEventToPool(TransitionToActiveEvent);
	FPlatformProcess::ReturnSynchEventToPool(SignalHeartbeatEvent);
	HeartbeatThread.Wait();

	if (LogFile)
	{
		delete LogFile;
		LogFile = nullptr;
	}
}

void FGSDKInternal::HeartbeatAsyncTaskFunction()
{
	while (KeepHeartbeatRunning)
	{
		if (SignalHeartbeatEvent->Wait(NextHeartbeatIntervalMs))
		{
			SignalHeartbeatEvent->Reset();
		}

		SendHeartbeat();
	}
}

void FGSDKInternal::OnReceiveHeartbeatResponse(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	Request->OnProcessRequestComplete().Unbind();
	
	if (!bConnectedSuccessfully || Response->GetResponseCode() >= 300)
	{
		UE_LOG(LogPlayfabGSDK, Error, TEXT("Received non-success code from Agent.  Status Code: %d Response Body: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
		return;
	}

	DecodeHeartbeatResponse(Response->GetContentAsString());
}

void FGSDKInternal::StartLog()
{
	FScopeLock ScopeLock(&ConfigMutex);

	const FString LogFileName = FString::Printf(TEXT("GSDK_output_%lld.txt"), FDateTime::Now().GetTicks());
	FString LogFolder = ConfigSettings[FPlayfabGSDKModule::LOG_FOLDER_KEY];

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (!FileManager.CreateDirectoryTree(*LogFolder))
	{
		LogFolder = "";
	}

	const FString GSDKLogPath = LogFolder + LogFileName;

	LogFile = FileManager.OpenWrite(*GSDKLogPath, true);
}

void FGSDKInternal::LogMessage(const FString& Message)
{
	FScopeLock ScopeLock(&LogMutex);

	const FString MessageWithNewLine = FString::Printf(TEXT("%s\n"), *Message);

	TArray<uint8> MessageBytes;
	MessageBytes.SetNumUninitialized(MessageWithNewLine.Len());
	StringToBytes(MessageWithNewLine, MessageBytes.GetData(), MessageBytes.Num());
	
	LogFile->Write(MessageBytes.GetData(), MessageBytes.Num());
	LogFile->Flush(true);
}

void FGSDKInternal::SendHeartbeat()
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	for (const auto& HttpHeader: HttpHeaders)
	{
		Request->SetHeader(HttpHeader.Key, HttpHeader.Value);
	}

	Request->SetURL(HeartbeatUrl);
	Request->SetVerb(TEXT("PATCH"));
	Request->SetContentAsString(EncodeHeartbeatRequest());
	Request->OnProcessRequestComplete().BindRaw(this, &FGSDKInternal::OnReceiveHeartbeatResponse);

	Request->ProcessRequest();
}

FString FGSDKInternal::EncodeHeartbeatRequest()
{
	TSharedPtr<FJsonObject> HeartbeatRequestJson = MakeShared<FJsonObject>();

	HeartbeatRequestJson->SetStringField(TEXT("CurrentGameState"), GameStateNames[static_cast<int32>(HeartbeatRequest.CurrentGameState)]);

	if (OnHealthCheck.IsBound())
	{
		HeartbeatRequest.IsGameHealthy = OnHealthCheck.Execute();
	}

	HeartbeatRequestJson->SetStringField(TEXT("CurrentGameHealth"), HeartbeatRequest.IsGameHealthy ? TEXT("Healthy") : TEXT("Unhealthy"));

	TArray<TSharedPtr<FJsonValue>> ConnectedPlayersJson;
	for (const FConnectedPlayer& ConnectedPlayer: HeartbeatRequest.ConnectedPlayers)
	{
		TSharedPtr<FJsonObject> ConnectedPlayerJsonObject = MakeShared<FJsonObject>();

		ConnectedPlayerJsonObject->SetStringField(TEXT("PlayerId"), ConnectedPlayer.PlayerId);
		
		TSharedPtr<FJsonValueObject> ConnectedPlayerJson = MakeShared<FJsonValueObject>(ConnectedPlayerJsonObject);
		ConnectedPlayersJson.Add(ConnectedPlayerJson);
	}

	HeartbeatRequestJson->SetArrayField(TEXT("CurrentPlayers"), ConnectedPlayersJson);

	FString HeartbeatRequestJsonString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&HeartbeatRequestJsonString);
	FJsonSerializer::Serialize(HeartbeatRequestJson.ToSharedRef(), Writer);

	return HeartbeatRequestJsonString;
}

void FGSDKInternal::DecodeHeartbeatResponse(const FString& ResponseJson)
{
	TSharedPtr<FJsonObject> HeartbeatResponseJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseJson);

	if (!FJsonSerializer::Deserialize(Reader, HeartbeatResponseJson))
	{
		UE_LOG(LogPlayfabGSDK, Error, TEXT("Failed to parse heartbeat"));
		return;
	}

	if (HeartbeatResponseJson->HasField(TEXT("sessionConfig")))
	{
		FScopeLock ScopeLock(&ConfigMutex);

		TSharedPtr<FJsonObject> SessionConfigJson = HeartbeatResponseJson->GetObjectField(TEXT("sessionConfig"));
		for (const auto& SessionConfigJsonValue: SessionConfigJson->Values)
		{
			FString ValueString;
			if (SessionConfigJsonValue.Value->TryGetString(ValueString))
			{
				if (ConfigSettings.Contains(SessionConfigJsonValue.Key))
				{
					ConfigSettings[SessionConfigJsonValue.Key] = ValueString;
				}
				else
				{
					ConfigSettings.Add(SessionConfigJsonValue.Key, ValueString);
				}
			}
		}		

		// Update initial players only if this is the first time populating it.
		if (InitialPlayers.Num() == 0 && SessionConfigJson->HasField(TEXT("initialPlayers")))
		{
			TArray<TSharedPtr<FJsonValue>> PlayersJson = SessionConfigJson->GetArrayField(TEXT("initialPlayers"));

			for (auto PlayerJson: PlayersJson)
			{
				InitialPlayers.Add(PlayerJson->AsString());
			}
		}

		if (SessionConfigJson->HasField(TEXT("metadata")))
		{
			TSharedPtr<FJsonObject> MetaDatasJson = SessionConfigJson->GetObjectField(TEXT("metadata"));

			for (const auto& MetaDataJson: MetaDatasJson->Values)
			{
				FString ValueString;
				if (MetaDataJson.Value->TryGetString(ValueString))
				{
					if (ConfigSettings.Contains(MetaDataJson.Key))
					{
						ConfigSettings[MetaDataJson.Key] = ValueString;
					}
					else
					{
						ConfigSettings.Add(MetaDataJson.Key, ValueString);
					}
				}
			}
		}
	}

	if (HeartbeatResponseJson->HasField("nextScheduledMaintenanceUtc"))
	{
		FDateTime NextMaintenance = ParseDate(HeartbeatResponseJson->GetStringField(TEXT("nextScheduledMaintenanceUtc")));
		FTimespan Diff = NextMaintenance - CachedScheduledMaintenance;

		if (OnMaintenance.IsBound() && (!Diff.IsZero() || CachedScheduledMaintenance.GetTicks()))
		{
			OnMaintenance.Execute(NextMaintenance);
			CachedScheduledMaintenance = NextMaintenance;
		}
	}

	if (HeartbeatResponseJson->HasField(TEXT("operation")))
	{
		EOperation NextOperation = OperationMap[HeartbeatResponseJson->GetStringField(TEXT("operation"))];

		switch (NextOperation)
		{
		case EOperation::Continue:
			{
				break;
			}
		case EOperation::Active:
			{
				if (HeartbeatRequest.CurrentGameState != EGameState::Active)
				{
					SetState(EGameState::Active);
					TransitionToActiveEvent->Trigger();
				}
				break;
			}
		case EOperation::Terminate:
			{
				if (HeartbeatRequest.CurrentGameState != EGameState::Terminating)
				{
					SetState(EGameState::Terminating);
					TransitionToActiveEvent->Trigger();
					AsyncTask(ENamedThreads::AnyThread, [this]()
					{
						if (this->OnShutdown.IsBound())
						{
							this->OnShutdown.Execute();
						}

						this->KeepHeartbeatRunning = false;
					});
				}
				break;
			}
		default:
			{
				UE_LOG(LogPlayfabGSDK, Error, TEXT("Unhandled operation received: %s"), OperationNames[static_cast<int32>(NextOperation)]);
				break;
			}
		}
	}

	if (HeartbeatResponseJson->HasField(TEXT("nextHeartbeatIntervalMs")))
	{
		NextHeartbeatIntervalMs = HeartbeatResponseJson->GetNumberField(TEXT("nextHeartbeatIntervalMs"));

		
		// Clamp to the minimum permitted interval.
		if (NextHeartbeatIntervalMs < 1000)
		{
			NextHeartbeatIntervalMs = 1000;
		}
	}
	else
	{
		NextHeartbeatIntervalMs = 1000;
	}
}

FDateTime FGSDKInternal::ParseDate(const FString& DateStr)
{
	FDateTime OutDateTime;
	return FDateTime::Parse(DateStr, OutDateTime);
}

void FGSDKInternal::SetState(EGameState State)
{
	FScopeLock ScopeLock(&StateMutex);

	if (HeartbeatRequest.CurrentGameState != State)
	{
		HeartbeatRequest.CurrentGameState = State;
		SignalHeartbeatEvent->Trigger();
	}
}

void FGSDKInternal::SetConnectedPlayers(const TArray<FConnectedPlayer>& CurrentConnectedPlayers)
{
	FScopeLock ScopeLock(&PlayersMutex);
	HeartbeatRequest.ConnectedPlayers = CurrentConnectedPlayers;
}

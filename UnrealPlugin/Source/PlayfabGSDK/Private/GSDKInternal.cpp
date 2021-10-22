// Copyright 2021 Stefan Krismann
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "GSDKInternal.h"

#include "GSDKConfiguration.h"
#include "GSDKInternalUtils.h"
#include "HttpModule.h"
#include "Async/Async.h"
#include "Json.h"
#include "PlayFabGSDK.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"
#include "HAL/Event.h"

FGSDKInternal::FGSDKInternal()
	: TransitionToActiveEvent(FPlatformProcess::GetSynchEventFromPool(false))
	, SignalHeartbeatEvent(FPlatformProcess::GetSynchEventFromPool(false))
{
	TUniquePtr<FConfiguration> ConfigPtr;

	FString FileName = UGSDKInternalUtils::GetEnvironmentVariable(TEXT("GSDK_CONFIG_FILE"));

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

	ConfigSettings.Add(FPlayFabGSDKModule::HEARTBEAT_ENDPOINT_KEY, ConfigPtr->GetHeartbeatEndpoint());
	ConfigSettings.Add(FPlayFabGSDKModule::SERVER_ID_KEY, ConfigPtr->GetServerId());
	ConfigSettings.Add(FPlayFabGSDKModule::LOG_FOLDER_KEY, ConfigPtr->GetLogFolder());
	ConfigSettings.Add(FPlayFabGSDKModule::SHARED_CONTENT_FOLDER_KEY, ConfigPtr->GetSharedContentFolder());
	ConfigSettings.Add(FPlayFabGSDKModule::CERTIFICATE_FOLDER_KEY, ConfigPtr->GetCertificateFolder());
	ConfigSettings.Add(FPlayFabGSDKModule::TITLE_ID_KEY, ConfigPtr->GetTitleId());
	ConfigSettings.Add(FPlayFabGSDKModule::BUILD_ID_KEY, ConfigPtr->GetBuildId());
	ConfigSettings.Add(FPlayFabGSDKModule::REGION_KEY, ConfigPtr->GetRegion());
	ConfigSettings.Add(FPlayFabGSDKModule::PUBLIC_IP_V4_ADDRESS_KEY, ConfigPtr->GetPublicIpV4Address());
	ConfigSettings.Add(FPlayFabGSDKModule::FULLY_QUALIFIED_DOMAIN_NAME_KEY, ConfigPtr->GetFullyQualifiedDomainName());

	if (ConfigSettings[FPlayFabGSDKModule::HEARTBEAT_ENDPOINT_KEY].IsEmpty() || ConfigSettings[FPlayFabGSDKModule::SERVER_ID_KEY].IsEmpty())
	{
		UE_LOG(LogPlayFabGSDK, Fatal, TEXT("Heartbeat endpoint and Server id are required configuration values."));
		return;
	}

	ConnectionInfo = ConfigPtr->GetGameServerConnectionInfo();

	if (ConfigPtr->ShouldLog())
	{
		StartLog();
	}

	// Use highest frequency permitted heartbeat interval until VMAgent tells an updated one.
	MinimumHeartbeatInterval = ConfigPtr->GetMinimumHeartbeatInterval();
	NextHeartbeatIntervalMs = MinimumHeartbeatInterval;

	MaximumUnexpectedOperationsErrorCount = ConfigPtr->GetMaximumAllowedUnexpectedOperationsCount();

	const FString GsmBaseUrl = ConfigSettings[FPlayFabGSDKModule::HEARTBEAT_ENDPOINT_KEY];
	const FString InstanceId = ConfigSettings[FPlayFabGSDKModule::SERVER_ID_KEY];

	UE_LOG(LogPlayFabGSDK, Log, TEXT("VM Agent Endpoint: %s"), *GsmBaseUrl);
	UE_LOG(LogPlayFabGSDK, Log, TEXT("Instance Id: %s"), *InstanceId);

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

	if (OutputDevice)
	{
		GLog->RemoveOutputDevice(OutputDevice);
		delete OutputDevice;
		OutputDevice = nullptr;
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

		if (KeepHeartbeatRunning)
		{
			SendHeartbeat();
			ReceiveHeartbeat();
		}
	}
}

void FGSDKInternal::StartLog()
{
	FScopeLock ScopeLock(&ConfigMutex);

	const FString LogFileName = FString::Printf(TEXT("GSDK_output_%lld.txt"), FDateTime::Now().GetTicks());
	FString LogFolder = ConfigSettings[FPlayFabGSDKModule::LOG_FOLDER_KEY];

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (!FileManager.CreateDirectoryTree(*LogFolder))
	{
		LogFolder = TEXT("");
	}

	const FString GSDKLogPath = FString::Printf(TEXT("%s/%s"), *LogFolder, *LogFileName);

	OutputDevice = new FOutputDeviceFile(*GSDKLogPath);
	GLog->AddOutputDevice(OutputDevice);
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

	HeartBeats.Add(Request);

	Request->ProcessRequest();
}

void FGSDKInternal::ReceiveHeartbeat()
{
	if (HeartBeats.Num() == 0 || !HeartBeats[0]->GetResponse().IsValid())
	{
		return;
	}

	auto Request = HeartBeats[0];
	auto Response = HeartBeats[0]->GetResponse();

	if (Response->GetContentAsString().Len() != Response->GetContentLength())
	{
		return;
	}

	HeartBeats.RemoveAt(0);

	if (Response->GetResponseCode() >= 300)
	{
		UE_LOG(LogPlayFabGSDK, Error, TEXT("Received non-success code from Agent.  Status Code: %d Response Body: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
		return;
	}

	DecodeHeartbeatResponse(Response->GetContentAsString());
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
		UE_LOG(LogPlayFabGSDK, Error, TEXT("Failed to parse heartbeat"));
		return;
	}

	if (HeartbeatResponseJson->HasField(TEXT("sessionConfig")))
	{
		TSharedPtr<FJsonObject> SessionConfigJson = HeartbeatResponseJson->GetObjectField(TEXT("sessionConfig"));

		// Update initial players only if this is the first time populating it.
		if (InitialPlayers.Num() == 0 && SessionConfigJson->HasField(TEXT("initialPlayers")))
		{
			TArray<TSharedPtr<FJsonValue>> PlayersJson = SessionConfigJson->GetArrayField(TEXT("initialPlayers"));

			for (auto PlayerJson : PlayersJson)
			{
				InitialPlayers.Add(PlayerJson->AsString());
			}
		}

		const bool hasConfigValues = SessionConfigJson->Values.Num() > 0;
		const bool hasMetaData = SessionConfigJson->HasField(TEXT("metadata"));

		if (hasConfigValues || hasMetaData)
		{
			FScopeLock ScopeLock(&ConfigMutex);

			for (const auto& SessionConfigJsonValue : SessionConfigJson->Values)
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

			if (hasMetaData)
			{
				TSharedPtr<FJsonObject> MetaDatasJson = SessionConfigJson->GetObjectField(TEXT("metadata"));

				for (const auto& MetaDataJson : MetaDatasJson->Values)
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
	}

	if (HeartbeatResponseJson->HasField(TEXT("nextScheduledMaintenanceUtc")))
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

		bool bWasOperationValid = true;

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
					TriggerShutdown();
				}
				break;
			}
		default:
			{
				UE_LOG(LogPlayFabGSDK, Error, TEXT("Unhandled operation received: %s"), OperationNames[static_cast<int32>(NextOperation)]);
				bWasOperationValid = false;
				break;
			}
		}

		if (!bWasOperationValid)
		{
			UnexpectedOperationsErrorCount++;

			// Too many unexpected operations have been sent, time to shut down
			if (UnexpectedOperationsErrorCount >= MaximumUnexpectedOperationsErrorCount)
			{
				UE_LOG(LogPlayFabGSDK, Error, TEXT("Too many unexpected operations received. Shutting down!"));
				TriggerShutdown();
			}
		}
		else
		{
			UnexpectedOperationsErrorCount = 0;
		}
	}

	if (HeartbeatResponseJson->HasField(TEXT("nextHeartbeatIntervalMs")))
	{
		NextHeartbeatIntervalMs = HeartbeatResponseJson->GetNumberField(TEXT("nextHeartbeatIntervalMs"));


		NextHeartbeatIntervalMs = FMath::Max(MinimumHeartbeatInterval, NextHeartbeatIntervalMs);
	}
	else
	{
		NextHeartbeatIntervalMs = MinimumHeartbeatInterval;
	}
}

FDateTime FGSDKInternal::ParseDate(const FString& DateStr)
{
	FDateTime OutDateTime;
	return FDateTime::ParseHttpDate(DateStr, OutDateTime);
}

void FGSDKInternal::TriggerShutdown()
{
	AsyncTask(ENamedThreads::AnyThread, [this]()
	{
		this->KeepHeartbeatRunning = false;

		for (auto Heartbeat: HeartBeats)
		{
			Heartbeat->CancelRequest();
		}

		HeartBeats.Empty();


		if (this->OnShutdown.IsBound())
		{
			this->OnShutdown.Execute();
		}
	});
}

FString FGSDKInternal::GetConfig(const FString& Key) const
{
	FScopeLock ScopeLock(&ConfigMutex);
	if (ConfigSettings.Contains(Key))
	{
		return ConfigSettings[Key];
	}
	return TEXT("");
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

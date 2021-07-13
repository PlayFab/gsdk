// Copyright Stefan Krismann
// MIT License

#include "GSDKConfiguration.h"

#include "GSDKUtils.h"
#include "PlayfabGSDK.h"
#include "Logging/LogMacros.h"
#include "Misc/FileHelper.h"

FConfigurationBase::FConfigurationBase()
{
	TitleId = UGSDKUtils::GetEnvironmentVariable(TITLE_ID_ENV_VAR);
	BuildId = UGSDKUtils::GetEnvironmentVariable(BUILD_ID_ENV_VAR);
	Region = UGSDKUtils::GetEnvironmentVariable(REGION_ENV_VAR);
}

const FString& FConfigurationBase::GetTitleId()
{
	return TitleId;
}

const FString& FConfigurationBase::GetBuildId()
{
	return BuildId;
}

const FString& FConfigurationBase::GetRegion()
{
	return Region;
}

bool FConfigurationBase::ShouldLog()
{
	return true;
}

bool FConfigurationBase::ShouldHeartbeat()
{
	return true;
}

FEnvironmentVariableConfiguration::FEnvironmentVariableConfiguration()
{
	HeartbeatEndpoint = UGSDKUtils::GetEnvironmentVariable(HEARTBEAT_ENDPOINT_ENV_VAR);
	ServerId = UGSDKUtils::GetEnvironmentVariable(SERVER_ID_ENV_VAR);
	LogFolder = UGSDKUtils::GetEnvironmentVariable(LOG_FOLDER_ENV_VAR);
	SharedContentFolder = UGSDKUtils::GetEnvironmentVariable(SHARED_CONTENT_FOLDER_ENV_VAR);

	// Game cert support was added once we switched to a json config, so we don't have values for them
}

const FString& FEnvironmentVariableConfiguration::GetHeartbeatEndpoint()
{
	return HeartbeatEndpoint;
}

const FString& FEnvironmentVariableConfiguration::GetServerId()
{
	return ServerId;
}

const FString& FEnvironmentVariableConfiguration::GetLogFolder()
{
	return LogFolder;
}

const FString& FEnvironmentVariableConfiguration::GetCertificateFolder()
{
	return CertFolder;
}

const FString& FEnvironmentVariableConfiguration::GetSharedContentFolder()
{
	return SharedContentFolder;
}

const TMap<FString, FString>& FEnvironmentVariableConfiguration::GetGameCertificates()
{
	return GameCerts;
}

const TMap<FString, FString>& FEnvironmentVariableConfiguration::GetBuildMetadata()
{
	return Metadata;
}

const TMap<FString, FString>& FEnvironmentVariableConfiguration::GetGamePorts()
{
	return Ports;
}

const FString& FEnvironmentVariableConfiguration::GetPublicIpV4Address()
{
	return Ipv4Address;
}

const FString& FEnvironmentVariableConfiguration::GetFullyQualifiedDomainName()
{
	return DomainName;
}

const FGameServerConnectionInfo& FEnvironmentVariableConfiguration::GetGameServerConnectionInfo()
{
	return ConnectionInfo;
}

FJsonFileConfiguration::FJsonFileConfiguration(const FString& FileName)
{
	if (!FPaths::FileExists(FileName))
	{
		UE_LOG(LogPlayfabGSDK, Fatal, TEXT("Specified configuration file doesnt exist: %s"), *FileName);
		return;
	}

	FString FileContent;

	if (!FFileHelper::LoadFileToString(FileContent, *FileName))
	{
		UE_LOG(LogPlayfabGSDK, Fatal, TEXT("Failed to read configuration file: %s"), *FileName);
		return;
	}

	TSharedPtr<FJsonObject> ConfigJson = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContent);
	if (!FJsonSerializer::Deserialize(JsonReader, ConfigJson))
	{
		UE_LOG(LogPlayfabGSDK, Fatal, TEXT("Failed to parse configuration file: %s"), *FileName);
		return;
	}

	HeartbeatEndpoint = ConfigJson->GetStringField("heartbeatEndpoint");
	ServerId = ConfigJson->GetStringField("sessionHostId");
	LogFolder = ConfigJson->GetStringField("logFolder");
	SharedContentFolder = ConfigJson->GetStringField("sharedContentFolder");

	if (ConfigJson->HasField("certificateFolder"))
	{
		CertFolder = ConfigJson->GetStringField("certificateFolder");
	}
	else
	{
		CertFolder = "";
	}

	if (ConfigJson->HasField("gameCertificates"))
	{
		TSharedPtr<FJsonObject> GameCertsJson = ConfigJson->GetObjectField("gameCertificates");
		for (const auto& GameCert: GameCertsJson->Values)
		{
			GameCerts.Add(GameCert.Key, GameCert.Value->AsString());
		}
	}

	if (ConfigJson->HasField("buildMetadata"))
	{
		TSharedPtr<FJsonObject> BuildMetaData = ConfigJson->GetObjectField("buildMetadata");
		for (const auto& BuildMetaDataValue: BuildMetaData->Values)
		{
			Metadata.Add(BuildMetaDataValue.Key, BuildMetaDataValue.Value->AsString());
		}
	}

	if (ConfigJson->HasField("gamePorts"))
	{
		TSharedPtr<FJsonObject> GamePortsJson = ConfigJson->GetObjectField("gamePorts");
		for (const auto& GamePort: GamePortsJson->Values)
		{
			Ports.Add(GamePort.Key, GamePort.Value->AsString());
		}
	}

	if (ConfigJson->HasField("publicIpV4Address"))
	{
		Ipv4Address = ConfigJson->GetStringField("publicIpV4Address");
	}

	if (ConfigJson->HasField("fullyQualifiedDomainName"))
	{
		DomainName = ConfigJson->GetStringField("fullyQualifiedDomainName");
	}

	if (ConfigJson->HasField("gameServerConnectionInfo"))
	{
		TSharedPtr<FJsonObject> ConnectionInfoJson = ConfigJson->GetObjectField("gameServerConnectionInfo");
		TArray<TSharedPtr<FJsonValue>> PortsConfiguration = ConnectionInfoJson->GetArrayField("gamePortsConfiguration");
		TArray<FGamePort> GamePorts;

		for (const auto& PortJson: PortsConfiguration)
		{
			auto PortObject = PortJson->AsObject();
			GamePorts.Add(FGamePort(PortObject->GetStringField("name"), PortObject->GetNumberField("serverListeningPort"), PortObject->GetNumberField("clientConnectionPort")));
		}

		ConnectionInfo = FGameServerConnectionInfo(ConnectionInfoJson->GetStringField("publicIpV4Adress"), GamePorts);
	}
}

const FString& FJsonFileConfiguration::GetHeartbeatEndpoint()
{
	return HeartbeatEndpoint;
}

const FString& FJsonFileConfiguration::GetServerId()
{
	return ServerId;
}

const FString& FJsonFileConfiguration::GetLogFolder()
{
	return LogFolder;
}

const FString& FJsonFileConfiguration::GetCertificateFolder()
{
	return CertFolder;
}

const FString& FJsonFileConfiguration::GetSharedContentFolder()
{
	return SharedContentFolder;
}

const TMap<FString, FString>& FJsonFileConfiguration::GetGameCertificates()
{
	return GameCerts;
}

const TMap<FString, FString>& FJsonFileConfiguration::GetBuildMetadata()
{
	return Metadata;
}

const TMap<FString, FString>& FJsonFileConfiguration::GetGamePorts()
{
	return Ports;
}

const FString& FJsonFileConfiguration::GetPublicIpV4Address()
{
	return Ipv4Address;
}

const FString& FJsonFileConfiguration::GetFullyQualifiedDomainName()
{
	return DomainName;
}

const FGameServerConnectionInfo& FJsonFileConfiguration::GetGameServerConnectionInfo()
{
	return ConnectionInfo;
}

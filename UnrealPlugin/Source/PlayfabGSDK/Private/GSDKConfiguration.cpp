// Copyright (C) Microsoft Corporation. All rights reserved.

#include "GSDKConfiguration.h"

#include "GSDKInternalUtils.h"
#include "PlayfabGSDK.h"
#include "Logging/LogMacros.h"
#include "Misc/FileHelper.h"

FConfigurationBase::FConfigurationBase()
{
	TitleId = UGSDKInternalUtils::GetEnvironmentVariable(TITLE_ID_ENV_VAR);
	BuildId = UGSDKInternalUtils::GetEnvironmentVariable(BUILD_ID_ENV_VAR);
	Region = UGSDKInternalUtils::GetEnvironmentVariable(REGION_ENV_VAR);
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

int32 FConfigurationBase::GetMinimumHeartbeatInterval()
{
	return 1000;
}

int32 FConfigurationBase::GetMaximumAllowedUnexpectedOperationsCount()
{
	return 100;
}

FEnvironmentVariableConfiguration::FEnvironmentVariableConfiguration()
{
	HeartbeatEndpoint = UGSDKInternalUtils::GetEnvironmentVariable(HEARTBEAT_ENDPOINT_ENV_VAR);
	ServerId = UGSDKInternalUtils::GetEnvironmentVariable(SERVER_ID_ENV_VAR);
	LogFolder = UGSDKInternalUtils::GetEnvironmentVariable(LOG_FOLDER_ENV_VAR);
	SharedContentFolder = UGSDKInternalUtils::GetEnvironmentVariable(SHARED_CONTENT_FOLDER_ENV_VAR);

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
		UE_LOG(LogPlayFabGSDK, Fatal, TEXT("Specified configuration file does not exist: %s"), *FileName);
		return;
	}

	FString FileContent;

	if (!FFileHelper::LoadFileToString(FileContent, *FileName))
	{
		UE_LOG(LogPlayFabGSDK, Fatal, TEXT("Failed to read configuration file: %s"), *FileName);
		return;
	}

	TSharedPtr<FJsonObject> ConfigJson = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContent);
	if (!FJsonSerializer::Deserialize(JsonReader, ConfigJson))
	{
		UE_LOG(LogPlayFabGSDK, Fatal, TEXT("Failed to parse configuration file: %s"), *FileName);
		return;
	}

	HeartbeatEndpoint = ConfigJson->GetStringField(TEXT("heartbeatEndpoint"));
	ServerId = ConfigJson->GetStringField(TEXT("sessionHostId"));
	LogFolder = ConfigJson->GetStringField(TEXT("logFolder"));
	SharedContentFolder = ConfigJson->GetStringField(TEXT("sharedContentFolder"));

	if (ConfigJson->HasField(TEXT("certificateFolder")))
	{
		CertFolder = ConfigJson->GetStringField(TEXT("certificateFolder"));
	}
	else
	{
		CertFolder = TEXT("");
	}

	if (ConfigJson->HasField(TEXT("gameCertificates")))
	{
		TSharedPtr<FJsonObject> GameCertsJson = ConfigJson->GetObjectField(TEXT("gameCertificates"));
		for (const auto& GameCert: GameCertsJson->Values)
		{
			GameCerts.Add(GameCert.Key, GameCert.Value->AsString());
		}
	}

	if (ConfigJson->HasField(TEXT("buildMetadata")))
	{
		TSharedPtr<FJsonObject> BuildMetaData = ConfigJson->GetObjectField(TEXT("buildMetadata"));
		for (const auto& BuildMetaDataValue: BuildMetaData->Values)
		{
			Metadata.Add(BuildMetaDataValue.Key, BuildMetaDataValue.Value->AsString());
		}
	}

	if (ConfigJson->HasField(TEXT("gamePorts")))
	{
		TSharedPtr<FJsonObject> GamePortsJson = ConfigJson->GetObjectField(TEXT("gamePorts"));
		for (const auto& GamePortCur: GamePortsJson->Values)
		{
			Ports.Add(GamePortCur.Key, GamePortCur.Value->AsString());
		}
	}

	if (ConfigJson->HasField(TEXT("publicIpV4Address")))
	{
		Ipv4Address = ConfigJson->GetStringField(TEXT("publicIpV4Address"));
	}

	if (ConfigJson->HasField(TEXT("fullyQualifiedDomainName")))
	{
		DomainName = ConfigJson->GetStringField(TEXT("fullyQualifiedDomainName"));
	}

	if (ConfigJson->HasField(TEXT("gameServerConnectionInfo")))
	{
		TSharedPtr<FJsonObject> ConnectionInfoJson = ConfigJson->GetObjectField(TEXT("gameServerConnectionInfo"));
		TArray<TSharedPtr<FJsonValue>> PortsConfiguration = ConnectionInfoJson->GetArrayField(TEXT("gamePortsConfiguration"));
		TArray<FGamePort> GamePorts;

		for (const auto& PortJson: PortsConfiguration)
		{
			auto PortObject = PortJson->AsObject();
			FGamePort Port;
			Port.Name = PortObject->GetStringField(TEXT("name"));
			Port.ServerListeningPort = PortObject->GetNumberField(TEXT("serverListeningPort"));
			Port.ClientConnectionPort = PortObject->GetNumberField(TEXT("clientConnectionPort"));
			GamePorts.Add(Port);
		}

		FGameServerConnectionInfo GameServerConnectionInfo;
		GameServerConnectionInfo.PublicIpV4Address = ConnectionInfoJson->GetStringField(TEXT("publicIpV4Adress"));
		GameServerConnectionInfo.GamePortsConfiguration = GamePorts;
		ConnectionInfo = FGameServerConnectionInfo(GameServerConnectionInfo);
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

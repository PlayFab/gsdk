// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameServerConnectionInfo.h"

class FConfiguration
{
public:
	virtual ~FConfiguration() = default;
	virtual const FString& GetHeartbeatEndpoint() = 0;
	virtual const FString& GetServerId() = 0;
	virtual const FString& GetLogFolder() = 0;
	virtual const FString& GetCertificateFolder() = 0;
	virtual const FString& GetSharedContentFolder() = 0;
	virtual const TMap<FString, FString>& GetGameCertificates() = 0;
	virtual const FString& GetTitleId() = 0;
	virtual const FString& GetBuildId() = 0;
	virtual const FString& GetRegion() = 0;
	virtual const TMap<FString, FString>& GetBuildMetadata() = 0;
	virtual const TMap<FString, FString>& GetGamePorts() = 0;
	virtual const FString& GetPublicIpV4Address() = 0;
	virtual const FString& GetFullyQualifiedDomainName() = 0;
	virtual const FGameServerConnectionInfo& GetGameServerConnectionInfo() = 0;
	virtual bool ShouldLog() = 0;
	virtual bool ShouldHeartbeat() = 0;
	virtual int32 GetMinimumHeartbeatInterval() = 0;
	virtual int32 GetMaximumAllowedUnexpectedOperationsCount() = 0;

protected:
	static constexpr const TCHAR* HEARTBEAT_ENDPOINT_ENV_VAR = TEXT("HEARTBEAT_ENDPOINT");
	static constexpr const TCHAR* SERVER_ID_ENV_VAR = TEXT("SESSION_HOST_ID");
	static constexpr const TCHAR* LOG_FOLDER_ENV_VAR = TEXT("GSDK_LOG_FOLDER");
	static constexpr const TCHAR* TITLE_ID_ENV_VAR = TEXT("PF_TITLE_ID");
	static constexpr const TCHAR* BUILD_ID_ENV_VAR = TEXT("PF_BUILD_ID");
	static constexpr const TCHAR* REGION_ENV_VAR = TEXT("PF_REGION");
	static constexpr const TCHAR* SHARED_CONTENT_FOLDER_ENV_VAR = TEXT("SHARED_CONTENT_FOLDER");
};

class FConfigurationBase : public FConfiguration
{
public:
	FConfigurationBase();

	virtual const FString& GetTitleId() override;
	virtual const FString& GetBuildId() override;
	virtual const FString& GetRegion() override;
	virtual bool ShouldLog() override;
	virtual bool ShouldHeartbeat() override;
	virtual int32 GetMinimumHeartbeatInterval() override;
	virtual int32 GetMaximumAllowedUnexpectedOperationsCount() override;

private:
	// These are set in the base class
	FString TitleId;
	FString BuildId;
	FString Region;
};

class FEnvironmentVariableConfiguration : public FConfigurationBase
{
public:
	FEnvironmentVariableConfiguration();

	virtual const FString& GetHeartbeatEndpoint() override;
	virtual const FString& GetServerId() override;
	virtual const FString& GetLogFolder() override;
	virtual const FString& GetCertificateFolder() override;
	virtual const FString& GetSharedContentFolder() override;
	virtual const TMap<FString, FString>& GetGameCertificates() override;
	virtual const TMap<FString, FString>& GetBuildMetadata() override;
	virtual const TMap<FString, FString>& GetGamePorts() override;
	virtual const FString& GetPublicIpV4Address() override;
	virtual const FString& GetFullyQualifiedDomainName() override;
	virtual const FGameServerConnectionInfo& GetGameServerConnectionInfo() override;

private:
	FString HeartbeatEndpoint;
	FString ServerId;
	FString LogFolder;
	FString CertFolder;
	FString SharedContentFolder;
	TMap<FString, FString> GameCerts;
	TMap<FString, FString> Metadata;
	TMap<FString, FString> Ports;
	FString Ipv4Address;
	FString DomainName;
	FGameServerConnectionInfo ConnectionInfo;
};

class FJsonFileConfiguration : public FConfigurationBase
{
public:
	FJsonFileConfiguration(const FString& FileName);

	virtual const FString& GetHeartbeatEndpoint() override;
	virtual const FString& GetServerId() override;
	virtual const FString& GetLogFolder() override;
	virtual const FString& GetCertificateFolder() override;
	virtual const FString& GetSharedContentFolder() override;
	virtual const TMap<FString, FString>& GetGameCertificates() override;
	virtual const TMap<FString, FString>& GetBuildMetadata() override;
	virtual const TMap<FString, FString>& GetGamePorts() override;
	virtual const FString& GetPublicIpV4Address() override;
	virtual const FString& GetFullyQualifiedDomainName() override;
	virtual const FGameServerConnectionInfo& GetGameServerConnectionInfo() override;

private:
	FString HeartbeatEndpoint;
	FString ServerId;
	FString LogFolder;
	FString CertFolder;
	FString SharedContentFolder;
	TMap<FString, FString> GameCerts;
	TMap<FString, FString> Metadata;
	TMap<FString, FString> Ports;
	FString Ipv4Address;
	FString DomainName;
	FGameServerConnectionInfo ConnectionInfo;
};

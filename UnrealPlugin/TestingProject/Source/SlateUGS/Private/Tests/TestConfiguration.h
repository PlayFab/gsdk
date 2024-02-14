// Copyright (C) Microsoft Corporation. All rights reserved.

#if (WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR)
#pragma once

#include "CoreMinimal.h"
#include "PlayFabGSDK/Private/GSDKConfiguration.h"

class TestConfiguration : public FJsonFileConfiguration
{
public:
	TestConfiguration() = default;
	~TestConfiguration() = default;

	void SetHeartbeatEndpoint(FString heartbeatEndpoint);
	void SetServerId(FString serverId);
	void SetLogFolder(FString logFolder);
	void SetCertificateFolder(FString certFolder);
	void SetSharedContentFolder(FString sharedContentFolder);
	void SetGameCertificates(TMap<FString, FString> gameCerts);
	void SetBuildMetadata(TMap<FString, FString> metadata);
	void SetGamePorts(TMap<FString, FString> ports);
	void SetPublicIpV4Address(FString ipv4Address);
	void SetFullyQualifiedDomainName(FString domainName);
	void SetGameServerConnectionInfo(FGameServerConnectionInfo connectionInfo);

	bool SerializeToFile(const FString& fileName);
};
#endif

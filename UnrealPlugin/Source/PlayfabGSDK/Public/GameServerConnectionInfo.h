// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameServerConnectionInfo.generated.h"


USTRUCT(BlueprintType)
struct FGamePort
{
	GENERATED_BODY()
public:
	/// <summary>
	/// The friendly name / identifier for the port, specified by the game developer in the Build configuration.
	/// </summary>
	UPROPERTY(BlueprintReadOnly)
	FString Name;

	/// <summary>
	/// The port at which the game server should listen on (maps externally to <see cref="m_clientConnectionPort" />).
	/// For process based servers, this is determined by Control Plane, based on the ports available on the VM.
	/// For containers, this is specified by the game developer in the Build configuration.
	/// </summary>
	UPROPERTY(BlueprintReadOnly)
	int32 ServerListeningPort = 0;

	/// <summary>
	/// The public port to which clients should connect (maps internally to <see cref="m_serverListeningPort" />).
	/// </summary>
	UPROPERTY(BlueprintReadOnly)
	int32 ClientConnectionPort = 0;
};

USTRUCT(BlueprintType)
struct FGameServerConnectionInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	FString PublicIpV4Address;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGamePort> GamePortsConfiguration;
};

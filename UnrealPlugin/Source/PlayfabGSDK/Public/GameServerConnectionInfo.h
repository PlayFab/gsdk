// Copyright Stefan Krismann
// MIT License

#pragma once

#include "CoreMinimal.h"


struct FGamePort
{
public:
	/// <summary>
	/// The friendly name / identifier for the port, specified by the game developer in the Build configuration.
	/// </summary>
	FString Name;

	/// <summary>
	/// The port at which the game server should listen on (maps externally to <see cref="m_clientConnectionPort" />).
	/// For process based servers, this is determined by Control Plane, based on the ports available on the VM.
	/// For containers, this is specified by the game developer in the Build configuration.
	/// </summary>
	int32 ServerListeningPort;

	/// <summary>
	/// The public port to which clients should connect (maps internally to <see cref="m_serverListeningPort" />).
	/// </summary>
	int32 ClientConnectionPort;

	FGamePort() {}

	FGamePort(const FString& InName, int InServerListeningPort, int InClientConnectionPort)
	{
		Name = InName;
		ServerListeningPort = InServerListeningPort;
		ClientConnectionPort = InClientConnectionPort;
	}
};

struct FGameServerConnectionInfo
{
public:
	FString PublicIpV4Address;
	TArray<FGamePort> GamePortsConfiguration;

	FGameServerConnectionInfo() {}

	FGameServerConnectionInfo(const FString& InPublicIpV4Address, TArray<FGamePort> InGamePortsConfiguration)
	{
		PublicIpV4Address = InPublicIpV4Address;
		GamePortsConfiguration = InGamePortsConfiguration;
	}
};

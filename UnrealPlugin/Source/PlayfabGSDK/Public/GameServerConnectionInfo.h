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

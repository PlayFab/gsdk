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

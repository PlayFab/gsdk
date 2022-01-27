﻿// Copyright 2021 Stefan Krismann
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

#include "ConnectedPlayer.h"
#include "GameServerConnectionInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GSDKUtils.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnGSDKShutdown_Dyn);
DECLARE_DYNAMIC_DELEGATE(FOnGSDKServerActive_Dyn);
DECLARE_DYNAMIC_DELEGATE(FOnGSDKReadyForPlayers_Dyn);
DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FOnGSDKHealthCheck_Dyn);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGSDKMaintenance_Dyn, const FDateTime&, MaintenanceTime);

/**
 *
 */
UCLASS()
class PLAYFABGSDK_API UGSDKUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Sets state to StandBy to mark end of server initialization. Name follows convention of GSDK where once
	// game initialization is complete, the game is ready for players and once MPS allocates the server they ll 
	// be able to connect.
	static void ReadyForPlayers();

	/// Gets information (ipAddress and ports) for connecting to the game server, as well as the ports the
	/// game server should listen on.
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FGameServerConnectionInfo GetGameServerConnectionInfo();

	/// Tells the PlayFab service information on who is connected.
	UFUNCTION(BlueprintCallable, Category="PlayFab|GSDK|Players")
	static void UpdateConnectedPlayers(const TArray<FConnectedPlayer>& CurrentlyConnectedPlayers);

	/// Returns a path to the directory where logs will be mapped to the VM host
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetLogsDirectory();

	/// Returns a path to the directory shared by all game servers to cache data.
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetSharedContentDirectory();

	/// Returns the match id of the server.
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetMatchId();

	/// Returns the match session cookie of the server.
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetMatchSessionCookie();

	/// Returns the title id of the title this server is hosted for.
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetTitleId();

	/// Returns the region name where this server is located.
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetRegionName();

	/// Returns the build id used for this server.
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetBuildId();

	/// Returns the VM id this server is running on.
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetVMId();

	/// Returns the server id identifying this server
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetServerId();

	/// Returns custom set meta data value set at build creation
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Server Info")
	static const FString GetMetaDataValue(const FString& MetaDataName);

	/// After allocation, returns a list of the initial players that have access to this game server, used by PlayFab's Matchmaking offering
	UFUNCTION(BlueprintPure, Category="PlayFab|GSDK|Players")
	static const TArray<FString> GetInitialPlayers();

	/// Register the GSDK Shutdown Delegate to get notified when the server gets shutdown by outside forces
	UFUNCTION(BlueprintCallable, Category="PlayFab|GSDK|Callbacks")
	static void RegisterGSDKShutdownDelegate(const FOnGSDKShutdown_Dyn& OnGSDKShutdownDelegate);

	/// Register the GSDK Transition To Active State Delegate to get notified when the server
	/// transitions from StandBy / Waiting to Active
	UFUNCTION(BlueprintCallable, Category = "PlayFab|GSDK|Callbacks")
	static void RegisterGSDKServerActiveDelegate(const FOnGSDKServerActive_Dyn& OnGSDKServerActiveDelegate);
	
	/// Register the GSDK GameServerInitializationComplete Delegate which sets the game status to Standby, marking that
	/// the game is ready for players since all initialization is complete.
	UFUNCTION(BlueprintCallable, Category="PlayFab|GSDK|Callbacks")
	static void RegisterGSDKReadyForPlayers(const FOnGSDKReadyForPlayers_Dyn& OnGSDKReadyForPlayersDelegate);


	/// Register the GSDK Health Delegate, which gets called on every heartbeat to ensure that the server is still healthy
	UFUNCTION(BlueprintCallable, Category="PlayFab|GSDK|Callbacks")
	static void RegisterGSDKHealthCheckDelegate(const FOnGSDKHealthCheck_Dyn& OnGSDKHealthCheckDelegate);

	// Register the GSDK Maintenance Delegate, which gets called if the VM this server is running on expects to go into Maintenance soon
	UFUNCTION(BlueprintCallable, Category="PlayFab|GSDK|Callbacks")
	static void RegisterGSDKMaintenanceDelegate(const FOnGSDKMaintenance_Dyn& OnGSDKMaintenanceDelegate);

	/// Read the port information from MPS, and assign the default server hosting port if applicable
	UFUNCTION(BlueprintPure, Category = "PlayFab|GSDK|SetDefaultServerHostPort")
	static bool SetDefaultServerHostPort();

protected:
	/// Returns all configuration settings
	/// Returns a map of configuration setting values
	static FString GetConfigValue(const FString& Key);
};

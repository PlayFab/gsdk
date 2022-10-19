// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "ConnectedPlayer.h"
#include "GameServerConnectionInfo.h"
#include "GSDKInternal.h"

#include "Modules/ModuleManager.h"
#include "Templates/UniquePtr.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayFabGSDK, Log, All);



class FPlayFabGSDKModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FPlayFabGSDKModule& Get() { return FModuleManager::LoadModuleChecked<FPlayFabGSDKModule>(TEXT("PlayFabGSDK"));}

	// Sets state to StandBy to mark end of server initialization. Name follows convention of GSDK where once
	// game initialization is complete, the game is ready for players and once MPS allocates the server they ll 
	// be able to connect.
	void ReadyForPlayers();

	/// <summary>
	/// Gets information (ipAddress and ports) for connecting to the game server, as well as the ports the
	/// game server should listen on.
	/// </summary>
	/// <returns></returns>
	const FGameServerConnectionInfo GetGameServerConnectionInfo();

	/// <summary>Returns all configuration settings</summary>
	/// <returns>unordered map of string key:value configuration setting values</returns>
	FString GetConfigValue(const FString& Key);

	/// <summary>Tells the Xcloud service information on who is connected.</summary>
	/// <param name="CurrentlyConnectedPlayers">Array of player ids</param>
	void UpdateConnectedPlayers(const TArray<FConnectedPlayer>& CurrentlyConnectedPlayers);

protected:
	DECLARE_DELEGATE(FOnShutdown);
	DECLARE_DELEGATE(FOnServerActive);
	DECLARE_DELEGATE(FOnReadyForPlayers);
	DECLARE_DELEGATE_RetVal(bool, FOnHealthCheck);
	DECLARE_DELEGATE_OneParam(FOnMaintenance, const FDateTime&)

public:
	/// <summary>Gets called if the server is shutting us down</summary>
	FOnShutdown OnShutdown;

	/// </summary>Gets called when the server moves to an active state</summary>
	FOnServerActive OnServerActive;	

	/// </summary>Gets called when the server is ready to move from initialization to standby</summary>
	FOnReadyForPlayers OnReadyForPlayers;

	/// <summary>Gets called when the agent needs to check on the game's health</summary>
	FOnHealthCheck OnHealthCheck;

	/// <summary>Gets called if the server is getting a scheduled maintenance, it will get the UTC time of the maintenance event as an argument.</summary>
	FOnMaintenance OnMaintenance;

	/// <summary>Returns a path to the directory where logs will be mapped to the VM host</summary>
	const FString GetLogsDirectory();

	/// <summary>Returns a path to the directory shared by all game servers to cache data.</summary>
	const FString GetSharedContentDirectory();

	/// <summary>After allocation, returns a list of the initial players that have access to this game server, used by PlayFab's Matchmaking offering</summary>
	const TArray<FString> GetInitialPlayers();

	/// <summary>Returns whether GSDK calls should be executed (if noGSDK flag was found then we are most likely running a local test server, so no MPS agent)</summary>
	const bool GetIsUsingGsdk() const { return !NoGSDK; }

	static constexpr const TCHAR* HEARTBEAT_ENDPOINT_KEY = TEXT("gsmsBaseUrl");
	static constexpr const TCHAR* SERVER_ID_KEY = TEXT("instanceId");
	static constexpr const TCHAR* LOG_FOLDER_KEY = TEXT("logFolder");
	static constexpr const TCHAR* SHARED_CONTENT_FOLDER_KEY = TEXT("sharedContentFolder");
	static constexpr const TCHAR* CERTIFICATE_FOLDER_KEY = TEXT("certificateFolder");
	static constexpr const TCHAR* TITLE_ID_KEY = TEXT("titleId");
	static constexpr const TCHAR* BUILD_ID_KEY = TEXT("buildId");
	static constexpr const TCHAR* REGION_KEY = TEXT("region");
	static constexpr const TCHAR* VM_ID_KEY = TEXT("vmId");
	static constexpr const TCHAR* PUBLIC_IP_V4_ADDRESS_KEY = TEXT("publicIpV4Address");
	static constexpr const TCHAR* FULLY_QUALIFIED_DOMAIN_NAME_KEY = TEXT("fullyQualifiedDomainName");

	// These two keys are only available after allocation
	static constexpr const TCHAR* SESSION_COOKIE_KEY = TEXT("sessionCookie");
	static constexpr const TCHAR* SESSION_ID_KEY = TEXT("sessionId";)

private:

	TUniquePtr<FGSDKInternal> GSDKInternal = nullptr;

	bool NoGSDK = false;
};

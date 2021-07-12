// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ConnectedPlayer.h"
#include "GameServerConnectionInfo.h"
#include "FGSDKInternal.h"
#include "GSDKOutputDevice.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayfabGSDK, Log, All);



class FPlayfabGSDKModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FPlayfabGSDKModule& Get() { return FModuleManager::LoadModuleChecked<FPlayfabGSDKModule>(TEXT("PlayfabGSDK"));}

	/// <summary>Renamed from WaitForSessionAssignment. Called when the game server is ready to accept clients.  If Start() hasn't been called by this point, it will be called implicitly here.</summary>
	/// <remarks>Required. This is a blocking call and will only return when this server is either allocated (a player is about to connect) or terminated.</remarks>
	/// <returns>True if the server is allocated (will receive players shortly). False if the server is terminated. </returns>
	bool ReadyForPlayers();

	/// <summary>
    /// Gets information (ipAddress and ports) for connecting to the game server, as well as the ports the
    /// game server should listen on.
    /// </summary>
    /// <returns></returns>
    const FGameServerConnectionInfo GetGameServerConnectionInfo();

    /// <summary>Returns all configuration settings</summary>
    /// <returns>unordered map of string key:value configuration setting values</returns>
    const TMap<FString, FString> GetConfigSettings();

    /// <summary>Kicks off communication threads, heartbeats, etc.  Called implicitly by ReadyForPlayers if not called beforehand.</summary>
    /// <param name="debugLogs">Enables outputting additional logs to the GSDK log file.</param>
    // void Start(bool debugLogs = false);

    /// <summary>Tells the Xcloud service information on who is connected.</summary>
    /// <param name="CurrentlyConnectedPlayers">Array of player ids</param>
    void UpdateConnectedPlayers(const TArray<FConnectedPlayer>& CurrentlyConnectedPlayers);


protected:
	DECLARE_DELEGATE(FOnShutdown);
	DECLARE_DELEGATE_RetVal(bool, FOnHealthCheck);
	DECLARE_DELEGATE_OneParam(FOnMaintenance, const FDateTime&)

public:
	/// <summary>Gets called if the server is shutting us down</summary>
	FOnShutdown OnShutdown;

	/// <summary>Gets called when the agent needs to check on the game's health</summary>
	FOnHealthCheck OnHealthCheck;

	/// <summary>Gets called if the server is getting a scheduled maintenance, it will get the UTC time of the maintenance event as an argument.</summary>
	FOnMaintenance OnMaintenance;
	
    /// <summary>outputs a message to the log</summary>
    void LogMessage(const FString& Message);

    /// <summary>Returns a path to the directory where logs will be mapped to the VM host</summary>
    const FString GetLogsDirectory();

    /// <summary>Returns a path to the directory shared by all game servers to cache data.</summary>
    const FString GetSharedContentDirectory();

    /// <summary>After allocation, returns a list of the initial players that have access to this game server, used by PlayFab's Matchmaking offering</summary>
    const TArray<FString> GetInitialPlayers();

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

	// These two keys are only available after allocation (once readyForPlayers returns true)
	static constexpr const TCHAR* SESSION_COOKIE_KEY = TEXT("sessionCookie");
	static constexpr const TCHAR* SESSION_ID_KEY = TEXT("sessionId";)

private:

	FGSDKInternal* GSDKInternal = nullptr;
	FGSDKOutputDevice* OutputDevice = nullptr;
};
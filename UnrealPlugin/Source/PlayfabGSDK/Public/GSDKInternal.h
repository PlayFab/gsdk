// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "ConnectedPlayer.h"
#include "GameServerConnectionInfo.h"
#include "HAL/Event.h"
#include "HAL/CriticalSection.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/OutputDeviceFile.h"

#define MAKE_ENUM(VAR) VAR,
#define MAKE_STRINGS(VAR) TEXT(#VAR),

#define GAME_STATES(DO) \
	DO( Invalid ) \
	DO( Initializing ) \
	DO( StandingBy ) \
	DO( Active ) \
	DO( Terminating ) \
	DO( Terminated ) \
	DO( Quarantined ) \

enum class EGameState // send to Agent
{
	GAME_STATES(MAKE_ENUM)
};

const TCHAR* const GameStateNames[] =
{
	GAME_STATES(MAKE_STRINGS)
};

#define GAME_OPERATIONS(DO) \
	DO( Invalid ) \
	DO( Continue ) \
	DO( GetManifest ) \
	DO( Quarantine ) \
	DO( Active ) \
	DO( Terminate ) \
	DO( Operation_Count ) \

enum class EOperation // get back from Agent
{
	GAME_OPERATIONS(MAKE_ENUM)
};

const TCHAR* const OperationNames[] =
{
	GAME_OPERATIONS(MAKE_STRINGS)
};

struct FSessionConfig
{
public:
	FString SessionId;
	FString SessionCookie;

	TMap<FString, FString> ToMap()
	{
		TMap<FString, FString> ReturnMap;
		ReturnMap.Add(TEXT("sessionId"), SessionId);
		ReturnMap.Add(TEXT("sessionCookie"), SessionCookie);
		return ReturnMap;
	}
};

struct FSessionHostHeartbeatInfo
{
public:
	FString CurrentGameState; // The current game state. For example - StandingBy, Active, etc.
	int32 CurrentGameHealth; // The last queried game host health status
	TArray<FConnectedPlayer> CurrentPlayers; // Keeps track of the current list of connected players
	int32 NextHeartbeatIntervalMs; // The number of milliseconds to wait before sending the next heartbeat.

	EOperation Operation; //The next operation the VM Agent wants us to take
	FSessionConfig SessionConfig; // The configuration sent down to the game host from Control Plane
	FDateTime NextScheduledMaintenanceUtc; // The next scheduled maintenance time from Azure, in UTC
};

struct FHeartbeatRequest
{
	FHeartbeatRequest()
	{
		CurrentGameState = EGameState::Initializing;
		IsGameHealthy = true;
	}

	volatile EGameState CurrentGameState;
	bool IsGameHealthy;
	TArray<FConnectedPlayer> ConnectedPlayers;
};


struct FHeartbeatResponse
{
	FHeartbeatResponse()
	{
		ErrorValue = 0;
		NextScheduledMaintenanceUtc = 0; // set to POSIX start time
	}

	FString Operation;
	uint32 NextHeartbeatMilliseconds;
	uint16 StatusCode;
	int32 ErrorValue;
	FString ErrorMessage;
	FString ServerState;
	FDateTime NextScheduledMaintenanceUtc;
};

class FGSDKInternal
{
public:
	// These must be public for unique_ptr to work
	FGSDKInternal();
	~FGSDKInternal();

	TArray<FString> GetInitialPlayers() const
	{
		return InitialPlayers;
	}

	FGameServerConnectionInfo GetConnectionInfo() const
	{
		return ConnectionInfo;
	}

	FHeartbeatRequest& GetHeartbeatRequest()
	{
		return HeartbeatRequest;
	}

	FString GetConfigValue(const FString& Key) const;
	void SetState(EGameState State);
	void SetConnectedPlayers(const TArray<FConnectedPlayer>& CurrentConnectedPlayers);

	// Sets state to StandBy to mark end of server initialization. Name follows convention of GSDK where once
	// game initialization is complete, the game is ready for players and once MPS allocates the server they ll 
	// be able to connect.
	void ReadyForPlayers();

	DECLARE_DELEGATE(FOnShutdown);
	DECLARE_DELEGATE(FOnServerActive);
	DECLARE_DELEGATE(FOnReadyForPlayers);
	DECLARE_DELEGATE_RetVal(bool, FOnHealthCheck);
	DECLARE_DELEGATE_OneParam(FOnMaintenance, const FDateTime&)

	FOnShutdown OnShutdown;
	FOnServerActive OnServerActive;
	FOnReadyForPlayers OnReadyForPlayers;
	FOnMaintenance OnMaintenance;
	FOnHealthCheck OnHealthCheck;
private:

	#define ADD_OPERATION_MAP(VAR) ReturnMap.Add(TEXT(#VAR), EOperation::VAR);
	static TMap<FString, EOperation> InitializeOperationMap()
	{
		TMap<FString, EOperation> ReturnMap;

		GAME_OPERATIONS(ADD_OPERATION_MAP)

		return ReturnMap;
	}

	// NOTE: Making this map non-static, because otherwise the heartbeat thread
	// will throw an access violation exception when the game server main loop returns
	// (As C++ starts getting rid of all statics)
	const TMap<FString, EOperation> OperationMap = InitializeOperationMap();
	int32 MaximumUnexpectedOperationsErrorCount;
	int32 UnexpectedOperationsErrorCount = 0;

	FString AgentEntpoint;
	FHeartbeatRequest HeartbeatRequest;
	FString SessionCookie;
	int32 MinimumHeartbeatInterval;
	FString HeartbeatUrl;
	TFuture<void> HeartbeatThread;
	FOutputDeviceFile* OutputDevice;


	FGameServerConnectionInfo ConnectionInfo;
	TMap<FString, FString> ConfigSettings;
	FDateTime CachedScheduledMaintenance;

	TAtomic<bool> KeepHeartbeatRunning;

	DECLARE_DELEGATE(FOnShutdownThread)

	FOnShutdownThread OnShutdownThread;

	TMap<FString, FString> HttpHeaders;
	FCriticalSection StateMutex;
	FCriticalSection PlayersMutex;

	FEvent* SignalHeartbeatEvent;

	TArray<FString> InitialPlayers;

	void HeartbeatAsyncTaskFunction();
	mutable FCriticalSection ConfigMutex;

	FCriticalSection HeartbeatMutex;
	TArray<TSharedRef<IHttpRequest, ESPMode::ThreadSafe>> Heartbeats;

	void StartLog();
	void SendHeartbeat();
	void ReceiveHeartbeat();

	// These two methods are used for unit testing as well as regular operation.
	FString EncodeHeartbeatRequest();
	void DecodeHeartbeatResponse(const FString& ResponseJson);
	int32 NextHeartbeatIntervalMs;

	FDateTime ParseDate(const FString& DateStr);


	void TriggerShutdown();
};

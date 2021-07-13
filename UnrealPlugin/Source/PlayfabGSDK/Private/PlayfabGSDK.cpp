// Copyright Stefan Krismann
// MIT License

#include "PlayfabGSDK.h"

#include "GSDKConfiguration.h"
#if UE_SERVER
#include "GSDKUtils.h"
#include "Misc/ScopeLock.h"
#endif

DEFINE_LOG_CATEGORY(LogPlayfabGSDK);

#define LOCTEXT_NAMESPACE "FPlayfabGSDKModule"

void FPlayfabGSDKModule::StartupModule()
{
#if UE_SERVER
	GSDKInternal = new FGSDKInternal();

	GSDKInternal->OnShutdown.BindLambda([this]()
	{
		if (OnShutdown.IsBound())
		{
			OnShutdown.Execute();
		}
	});
	GSDKInternal->OnHealthCheck.BindLambda([this]()
	{
		if (OnHealthCheck.IsBound())
		{
			return OnHealthCheck.Execute();
		}

		return false;
	});
	GSDKInternal->OnMaintenance.BindLambda([this](const FDateTime& Time)
	{
		if (OnMaintenance.IsBound())
		{
			OnMaintenance.Execute(Time);
		}
	});
#endif
}

void FPlayfabGSDKModule::ShutdownModule()
{
#if UE_SERVER
	if (GSDKInternal)
	{
		delete GSDKInternal;
		GSDKInternal = nullptr;
	}
#endif
}

bool FPlayfabGSDKModule::ReadyForPlayers()
{
#if UE_SERVER
	if (GSDKInternal->GetHeartbeatRequest().CurrentGameState != EGameState::Active)
	{
		GSDKInternal->SetState(EGameState::StandingBy);
		GSDKInternal->GetTransitionToActiveEvent()->Wait();
	}

	return GSDKInternal->GetHeartbeatRequest().CurrentGameState == EGameState::Active;
#else
	return true;
#endif
}

const FGameServerConnectionInfo FPlayfabGSDKModule::GetGameServerConnectionInfo()
{
#if UE_SERVER
	return GSDKInternal->GetConnectionInfo();
#else
	return FGameServerConnectionInfo();
#endif
}

const TMap<FString, FString> FPlayfabGSDKModule::GetConfigSettings()
{
#if UE_SERVER
	return GSDKInternal->GetConfigSettings();
#else
	return TMap<FString, FString>();
#endif
}

void FPlayfabGSDKModule::UpdateConnectedPlayers(const TArray<FConnectedPlayer>& CurrentlyConnectedPlayers)
{
#if UE_SERVER
	GSDKInternal->SetConnectedPlayers(CurrentlyConnectedPlayers);
#endif
}

const FString FPlayfabGSDKModule::GetLogsDirectory()
{
#if UE_SERVER
	FScopeLock ScopeLock(&GSDKInternal->GetConfigMutex());

	const TMap<FString, FString> Config = GSDKInternal->GetConfigSettings();

	if (Config.Contains(LOG_FOLDER_KEY))
	{
		return Config[LOG_FOLDER_KEY];
	}
#endif

	return TEXT("");
}

const FString FPlayfabGSDKModule::GetSharedContentDirectory()
{
#if UE_SERVER
	FScopeLock ScopeLock(&GSDKInternal->GetConfigMutex());

	const TMap<FString, FString> Config = GSDKInternal->GetConfigSettings();

	if (Config.Contains(SHARED_CONTENT_FOLDER_KEY))
	{
		return Config[SHARED_CONTENT_FOLDER_KEY];
	}
#endif

	return TEXT("");
}

const TArray<FString> FPlayfabGSDKModule::GetInitialPlayers()
{
#if UE_SERVER
	return GSDKInternal->GetInitialPlayers();
#else
	return TArray<FString>();
#endif	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPlayfabGSDKModule, PlayfabGSDK)
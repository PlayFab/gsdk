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

#include "PlayFabGSDK.h"

#include "GSDKConfiguration.h"
#if UE_SERVER
#include "GSDKUtils.h"
#include "Misc/ScopeLock.h"
#endif

DEFINE_LOG_CATEGORY(LogPlayFabGSDK);

#define LOCTEXT_NAMESPACE "FPlayFabGSDKModule"

void FPlayFabGSDKModule::StartupModule()
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

void FPlayFabGSDKModule::ShutdownModule()
{
#if UE_SERVER
	if (GSDKInternal)
	{
		delete GSDKInternal;
		GSDKInternal = nullptr;
	}
#endif
}

bool FPlayFabGSDKModule::ReadyForPlayers()
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

const FGameServerConnectionInfo FPlayFabGSDKModule::GetGameServerConnectionInfo()
{
#if UE_SERVER
	return GSDKInternal->GetConnectionInfo();
#else
	return FGameServerConnectionInfo();
#endif
}

const TMap<FString, FString> FPlayFabGSDKModule::GetConfigSettings()
{
#if UE_SERVER
	return GSDKInternal->GetConfigSettings();
#else
	return TMap<FString, FString>();
#endif
}

void FPlayFabGSDKModule::UpdateConnectedPlayers(const TArray<FConnectedPlayer>& CurrentlyConnectedPlayers)
{
#if UE_SERVER
	GSDKInternal->SetConnectedPlayers(CurrentlyConnectedPlayers);
#endif
}

const FString FPlayFabGSDKModule::GetLogsDirectory()
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

const FString FPlayFabGSDKModule::GetSharedContentDirectory()
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

const TArray<FString> FPlayFabGSDKModule::GetInitialPlayers()
{
#if UE_SERVER
	return GSDKInternal->GetInitialPlayers();
#else
	return TArray<FString>();
#endif	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPlayFabGSDKModule, PlayFabGSDK)
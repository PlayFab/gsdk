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

#include "PlayfabGSDK.h"

#if UE_SERVER
	#define PLAYFAB_GSDK_SERVER true
#else
	#define PLAYFAB_GSDK_SERVER false
#endif



#include "GSDKConfiguration.h"
#if PLAYFAB_GSDK_SERVER
#include "GSDKUtils.h"
#include "Misc/ScopeLock.h"
#endif

DEFINE_LOG_CATEGORY(LogPlayFabGSDK);

#define LOCTEXT_NAMESPACE "FPlayFabGSDKModule"

void FPlayFabGSDKModule::StartupModule()
{
#if PLAYFAB_GSDK_SERVER
	GSDKInternal = MakeUnique<FGSDKInternal>();

	GSDKInternal->OnShutdown.BindLambda([this]()
	{
		if (OnShutdown.IsBound())
		{
			OnShutdown.Execute();
		}
	});
	GSDKInternal->OnServerActive.BindLambda([this]()
	{
		if (OnServerActive.IsBound())
		{
			OnServerActive.Execute();
		}
	});
	GSDKInternal->OnReadyForPlayers.BindLambda([this]()
	{
		if (OnReadyForPlayers.IsBound())
		{
			OnReadyForPlayers.Execute();
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
#if PLAYFAB_GSDK_SERVER
#endif
}

void FPlayFabGSDKModule::ReadyForPlayers()
{
#if PLAYFAB_GSDK_SERVER
	GSDKInternal->ReadyForPlayers();
#endif
}

const FGameServerConnectionInfo FPlayFabGSDKModule::GetGameServerConnectionInfo()
{
#if PLAYFAB_GSDK_SERVER
	return GSDKInternal->GetConnectionInfo();
#else
	return FGameServerConnectionInfo();
#endif
}

FString FPlayFabGSDKModule::GetConfigValue(const FString& Key)
{
#if PLAYFAB_GSDK_SERVER
	return GSDKInternal->GetConfigValue(Key);
#else
	return TEXT("");
#endif
}

void FPlayFabGSDKModule::UpdateConnectedPlayers(const TArray<FConnectedPlayer>& CurrentlyConnectedPlayers)
{
#if PLAYFAB_GSDK_SERVER
	GSDKInternal->SetConnectedPlayers(CurrentlyConnectedPlayers);
#endif
}

const FString FPlayFabGSDKModule::GetLogsDirectory()
{
#if PLAYFAB_GSDK_SERVER
	return GSDKInternal->GetConfigValue(LOG_FOLDER_KEY);
#endif
	return TEXT("");
}

const FString FPlayFabGSDKModule::GetSharedContentDirectory()
{
#if PLAYFAB_GSDK_SERVER
	return GSDKInternal->GetConfigValue(SHARED_CONTENT_FOLDER_KEY);
#endif

	return TEXT("");
}

const TArray<FString> FPlayFabGSDKModule::GetInitialPlayers()
{
#if PLAYFAB_GSDK_SERVER
	return GSDKInternal->GetInitialPlayers();
#else
	return TArray<FString>();
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPlayFabGSDKModule, PlayFabGSDK)

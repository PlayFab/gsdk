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

#include "GSDKUtils.h"

#include "PlayfabGSDK.h"

const FGameServerConnectionInfo UGSDKUtils::GetGameServerConnectionInfo()
{
	return FPlayFabGSDKModule::Get().GetGameServerConnectionInfo();
}

void UGSDKUtils::UpdateConnectedPlayers(const TArray<FConnectedPlayer>& CurrentlyConnectedPlayers)
{
	return FPlayFabGSDKModule::Get().UpdateConnectedPlayers(CurrentlyConnectedPlayers);
}

const FString UGSDKUtils::GetLogsDirectory()
{
	return FPlayFabGSDKModule::Get().GetLogsDirectory();
}

const FString UGSDKUtils::GetSharedContentDirectory()
{
	return FPlayFabGSDKModule::Get().GetSharedContentDirectory();
}

const FString UGSDKUtils::GetMatchId()
{
	return GetConfigValue(FPlayFabGSDKModule::SESSION_ID_KEY);
}

const FString UGSDKUtils::GetMatchSessionCookie()
{
	return GetConfigValue(FPlayFabGSDKModule::SESSION_COOKIE_KEY);
}

const FString UGSDKUtils::GetTitleId()
{
	return GetConfigValue(FPlayFabGSDKModule::TITLE_ID_KEY);
}

const FString UGSDKUtils::GetRegionName()
{
	return GetConfigValue(FPlayFabGSDKModule::REGION_KEY);
}

const FString UGSDKUtils::GetBuildId()
{
	return GetConfigValue(FPlayFabGSDKModule::BUILD_ID_KEY);
}

const FString UGSDKUtils::GetVMId()
{
	return GetConfigValue(FPlayFabGSDKModule::VM_ID_KEY);
}

const FString UGSDKUtils::GetServerId()
{
	return GetConfigValue(FPlayFabGSDKModule::SERVER_ID_KEY);
}

const FString UGSDKUtils::GetMetaDataValue(const FString& MetaDataName)
{
	return GetConfigValue(MetaDataName);
}

const TArray<FString> UGSDKUtils::GetInitialPlayers()
{
	return FPlayFabGSDKModule::Get().GetInitialPlayers();
}

void UGSDKUtils::RegisterGSDKShutdownDelegate(const FOnGSDKShutdown_Dyn& OnGSDKShutdownDelegate)
{
	if (FPlayFabGSDKModule::Get().OnMaintenance.IsBound())
	{
		UE_LOG(LogPlayFabGSDK, Error, TEXT("GSDK Shutdown Delegate is already bound! Will unbind the old binding!"));
	}

	FPlayFabGSDKModule::Get().OnShutdown.Unbind();
	FPlayFabGSDKModule::Get().OnShutdown.BindLambda([OnGSDKShutdownDelegate]()
	{
		if (OnGSDKShutdownDelegate.IsBound())
		{
			OnGSDKShutdownDelegate.Execute();
		}
	});
}

void UGSDKUtils::RegisterGSDKServerActiveDelegate(const FOnGSDKServerActive_Dyn& OnGSDKServerActiveDelegate)
{
	if (FPlayFabGSDKModule::Get().OnServerActive.IsBound())
	{
		UE_LOG(LogPlayFabGSDK, Error, TEXT("GSDK ServerActive Delegate is already bound! Will unbind the old binding!"));
	}

	FPlayFabGSDKModule::Get().OnServerActive.Unbind();
	FPlayFabGSDKModule::Get().OnServerActive.BindLambda([OnGSDKServerActiveDelegate]()
	{
		if (OnGSDKServerActiveDelegate.IsBound())
		{
			OnGSDKServerActiveDelegate.Execute();
		}
	});
}


void UGSDKUtils::RegisterGSDKOnReadyForPlayers(const FOnGSDKReadyForPlayers_Dyn& OnGSDKReadyForPlayersDelegate)
{
	if (FPlayFabGSDKModule::Get().OnReadyForPlayers.IsBound())
	{
		UE_LOG(LogPlayFabGSDK, Error, TEXT("GSDK ReadyForPlayers Delegate is already bound! Will unbind the old binding!"));
	}

	FPlayFabGSDKModule::Get().OnReadyForPlayers.Unbind();
	FPlayFabGSDKModule::Get().OnReadyForPlayers.BindLambda([OnGSDKReadyForPlayersDelegate]()
	{
		if (OnGSDKReadyForPlayersDelegate.IsBound())
		{
			OnGSDKReadyForPlayersDelegate.Execute();
	    }
	});
}

void UGSDKUtils::RegisterGSDKHealthCheckDelegate(const FOnGSDKHealthCheck_Dyn& OnGSDKHealthCheckDelegate)
{
	if (FPlayFabGSDKModule::Get().OnHealthCheck.IsBound())
	{
		UE_LOG(LogPlayFabGSDK, Error, TEXT("GSDK Health Delegate is already bound! Will unbind the old binding!"));
	}

	FPlayFabGSDKModule::Get().OnHealthCheck.Unbind();
	FPlayFabGSDKModule::Get().OnHealthCheck.BindLambda([OnGSDKHealthCheckDelegate]()
	{
		if (OnGSDKHealthCheckDelegate.IsBound())
		{
			return OnGSDKHealthCheckDelegate.Execute();
		}

		return false;
	});
}

void UGSDKUtils::RegisterGSDKMaintenanceDelegate(const FOnGSDKMaintenance_Dyn& OnGSDKMaintenanceDelegate)
{
	if (FPlayFabGSDKModule::Get().OnMaintenance.IsBound())
	{
		UE_LOG(LogPlayFabGSDK, Error, TEXT("GSDK Maintenance Delegate is already bound! Will unbind the old binding!"));
	}

	FPlayFabGSDKModule::Get().OnMaintenance.Unbind();
	FPlayFabGSDKModule::Get().OnMaintenance.BindLambda([OnGSDKMaintenanceDelegate](const FDateTime& MaintenanceTime)
	{
		if (OnGSDKMaintenanceDelegate.IsBound())
		{
			OnGSDKMaintenanceDelegate.Execute(MaintenanceTime);
		}
	});
}

FString UGSDKUtils::GetConfigValue(const FString& Key)
{
	return FPlayFabGSDKModule::Get().GetConfigValue(Key);
}

bool UGSDKUtils::SetDefaultServerHostPort()
{
	auto ConnectionInfo = GetGameServerConnectionInfo();
	// Override the internal Unreal Game Server hosting port number, with one driven from Game Manager
	// A game server hosted in MPS won't be accessible by clients unless it uses the port that MPS has configured for it.
	int32 UnrealServerGsdkHostPort = -1;
	for (auto& GamePorts : ConnectionInfo.GamePortsConfiguration) {
		// "UnrealServerGsdkHostPort" is documented as the name of a required port when using the internal Unreal Game Server hosting mechanism
		if (GamePorts.Name == TEXT("UnrealServerGsdkHostPort")) {
			UE_LOG(LogPlayFabGSDK, Warning, TEXT("GSDK Game server listening port: %d"), GamePorts.ServerListeningPort);
			UE_LOG(LogPlayFabGSDK, Warning, TEXT("GSDK Game client connection port: %d"), GamePorts.ClientConnectionPort);
			UnrealServerGsdkHostPort = GamePorts.ServerListeningPort;
			break;
		}
	}
	if (UnrealServerGsdkHostPort == -1)
	{
		UE_LOG(LogPlayFabGSDK, Fatal, TEXT("Unreal GSDK requires a named port: UnrealServerGsdkHostPort. This was not provided by MPS build port-configuration."));
		return false;
	}
	UE_LOG(LogPlayFabGSDK, Warning, TEXT("Assigning Unreal Server Host Port to MPS port: %d"), UnrealServerGsdkHostPort);
	FURL::UrlConfig.DefaultPort = UnrealServerGsdkHostPort;
	return true;
}

void UGSDKUtils::ReadyForPlayers()
{
	FPlayFabGSDKModule::Get().ReadyForPlayers();
}


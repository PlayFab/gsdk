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

#include "PlayFabGSDK.h"

bool UGSDKUtils::ReadyForPlayers()
{
	return FPlayFabGSDKModule::Get().ReadyForPlayers();
}

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
	const TMap<FString, FString> Config = GetConfigSettings();

	if (Config.Contains(FPlayFabGSDKModule::SESSION_ID_KEY))
	{
		return Config[FPlayFabGSDKModule::SESSION_ID_KEY];
	}

	return TEXT("");
}

const FString UGSDKUtils::GetMatchSessionCookie()
{
	const TMap<FString, FString> Config = GetConfigSettings();

	if (Config.Contains(FPlayFabGSDKModule::SESSION_COOKIE_KEY))
	{
		return Config[FPlayFabGSDKModule::SESSION_COOKIE_KEY];
	}

	return TEXT("");
}

const FString UGSDKUtils::GetTitleId()
{
	const TMap<FString, FString> Config = GetConfigSettings();

	if (Config.Contains(FPlayFabGSDKModule::TITLE_ID_KEY))
	{
		return Config[FPlayFabGSDKModule::TITLE_ID_KEY];
	}

	return TEXT("");
}

const FString UGSDKUtils::GetRegionName()
{
	const TMap<FString, FString> Config = GetConfigSettings();

	if (Config.Contains(FPlayFabGSDKModule::REGION_KEY))
	{
		return Config[FPlayFabGSDKModule::REGION_KEY];
	}

	return TEXT("");
}

const FString UGSDKUtils::GetBuildId()
{
	const TMap<FString, FString> Config = GetConfigSettings();

	if (Config.Contains(FPlayFabGSDKModule::BUILD_ID_KEY))
	{
		return Config[FPlayFabGSDKModule::BUILD_ID_KEY];
	}

	return TEXT("");
}

const FString UGSDKUtils::GetVMId()
{
	const TMap<FString, FString> Config = GetConfigSettings();

	if (Config.Contains(FPlayFabGSDKModule::VM_ID_KEY))
	{
		return Config[FPlayFabGSDKModule::VM_ID_KEY];
	}

	return TEXT("");
}

const FString UGSDKUtils::GetServerId()
{
	const TMap<FString, FString> Config = GetConfigSettings();

	if (Config.Contains(FPlayFabGSDKModule::SERVER_ID_KEY))
	{
		return Config[FPlayFabGSDKModule::SERVER_ID_KEY];
	}

	return TEXT("");
}

const FString UGSDKUtils::GetMetaDataValue(const FString& MetaDataName)
{
	const TMap<FString, FString> Config = GetConfigSettings();

	if (Config.Contains(MetaDataName))
	{
		return Config[MetaDataName];
	}

	return TEXT("");
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

const TMap<FString, FString> UGSDKUtils::GetConfigSettings()
{
	return FPlayFabGSDKModule::Get().GetConfigSettings();
}
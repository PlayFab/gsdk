// Copyright (C) Microsoft Corporation. All rights reserved.

#include "PlayFabGSDK.h"

#if UE_SERVER || (WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR)
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
#if (WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR)
{
}

void FPlayFabGSDKModule::ManualStartupModule()
#endif
{
#if PLAYFAB_GSDK_SERVER
	if (FParse::Param(FCommandLine::Get(), TEXT("noGSDK")) || FParse::Param(FCommandLine::Get(), TEXT("nogsdk")))
	{
		UE_LOG(LogPlayFabGSDK, Display, TEXT("Found noGSDK flag in launch args, no GSDK calls will be made."));
		NoGSDK = true;
		return;
	}
	
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

		return true;
	});
	GSDKInternal->OnMaintenance.BindLambda([this](const FDateTime& Time)
	{
		if (OnMaintenance.IsBound())
		{
			OnMaintenance.Execute(Time);
		}
	});
	GSDKInternal->OnMaintenanceV2.BindLambda([this](const FMaintenanceSchedule& schedule)
	{
		if (OnMaintenanceV2.IsBound())
		{
			OnMaintenanceV2.Execute(schedule);
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
	if (NoGSDK) return;
	GSDKInternal->ReadyForPlayers();
#endif
}

const FGameServerConnectionInfo FPlayFabGSDKModule::GetGameServerConnectionInfo()
{
#if PLAYFAB_GSDK_SERVER
	if (NoGSDK) return FGameServerConnectionInfo();
	return GSDKInternal->GetConnectionInfo();
#else
	return FGameServerConnectionInfo();
#endif
}

FString FPlayFabGSDKModule::GetConfigValue(const FString& Key)
{
#if PLAYFAB_GSDK_SERVER
	if (NoGSDK) return TEXT("");
	return GSDKInternal->GetConfigValue(Key);
#else
	return TEXT("");
#endif
}

void FPlayFabGSDKModule::UpdateConnectedPlayers(const TArray<FConnectedPlayer>& CurrentlyConnectedPlayers)
{
#if PLAYFAB_GSDK_SERVER
	if (NoGSDK) return;
	GSDKInternal->SetConnectedPlayers(CurrentlyConnectedPlayers);
#endif
}

const FString FPlayFabGSDKModule::GetLogsDirectory()
{
#if PLAYFAB_GSDK_SERVER
	if (NoGSDK) return TEXT("");
	return GSDKInternal->GetConfigValue(LOG_FOLDER_KEY);
#else
	return TEXT("");
#endif
}

const FString FPlayFabGSDKModule::GetSharedContentDirectory()
{
#if PLAYFAB_GSDK_SERVER
	if (NoGSDK) return TEXT("");
	return GSDKInternal->GetConfigValue(SHARED_CONTENT_FOLDER_KEY);
#else
	return TEXT("");
#endif
}

const TArray<FString> FPlayFabGSDKModule::GetInitialPlayers()
{
#if PLAYFAB_GSDK_SERVER
	if (NoGSDK) return TArray<FString>();
	return GSDKInternal->GetInitialPlayers();
#else
	return TArray<FString>();
#endif
}

#if (WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR)
void FPlayFabGSDKModule::ResetInternalState()
{
	GSDKInternal.Reset(nullptr);

	OnShutdown.Unbind();
	OnServerActive.Unbind();
	OnReadyForPlayers.Unbind();
	OnHealthCheck.Unbind();
	OnMaintenance.Unbind();
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPlayFabGSDKModule, PlayFabGSDK)

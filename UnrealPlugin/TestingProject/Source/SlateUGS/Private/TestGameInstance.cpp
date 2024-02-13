// Fill out your copyright notice in the Description page of Project Settings.


#include "TestGameInstance.h"
#include "PlayFabGSDK.h"
#include "GSDKUtils.h"

DEFINE_LOG_CATEGORY(LogPlayFabGSDKGameInstance);

void UTestGameInstance::Init()
{
    FOnGSDKShutdown_Dyn OnGsdkShutdown;
    OnGsdkShutdown.BindDynamic(this, &UTestGameInstance::OnGSDKShutdown);
    FOnGSDKHealthCheck_Dyn OnGsdkHealthCheck;
    OnGsdkHealthCheck.BindDynamic(this, &UTestGameInstance::OnGSDKHealthCheck);
    FOnGSDKServerActive_Dyn OnGSDKServerActive;
    OnGSDKServerActive.BindDynamic(this, &UTestGameInstance::OnGSDKServerActive);
    FOnGSDKReadyForPlayers_Dyn OnGSDKReadyForPlayers;
    OnGSDKReadyForPlayers.BindDynamic(this, &UTestGameInstance::OnGSDKReadyForPlayers);

    UGSDKUtils::RegisterGSDKShutdownDelegate(OnGsdkShutdown);
    UGSDKUtils::RegisterGSDKHealthCheckDelegate(OnGsdkHealthCheck);
    UGSDKUtils::RegisterGSDKServerActiveDelegate(OnGSDKServerActive);
    UGSDKUtils::RegisterGSDKReadyForPlayers(OnGSDKReadyForPlayers);

#if UE_SERVER
    UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Reached Init!"));
    UGSDKUtils::SetDefaultServerHostPort();
#endif
}

void UTestGameInstance::OnStart()
{
    UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Reached onStart!"));
    UGSDKUtils::ReadyForPlayers();
}

void UTestGameInstance::OnGSDKShutdown()
{
    UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Shutdown!"));
    FPlatformMisc::RequestExit(false);
}

bool UTestGameInstance::OnGSDKHealthCheck()
{
    UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Healthy!"));
    return true;
}

void UTestGameInstance::OnGSDKServerActive()
{
    /**
     * Server is transitioning to an active state.
     * Optional: Add in the implementation any code that is needed for the game server when
     * this transition occurs.
     */
    UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Active!"));
}

void UTestGameInstance::OnGSDKReadyForPlayers()
{
    /**
     * Server is transitioning to a StandBy state. Game initialization is complete and the game is ready
     * to accept players.
     * Optional: Add in the implementation any code that is needed for the game server before
     * initialization completes.
     */
    UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Finished Initialization - Moving to StandBy!"));
}
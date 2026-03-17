// Fill out your copyright notice in the Description page of Project Settings.


#include "TestGameInstanceGSDK.h"

DEFINE_LOG_CATEGORY(LogPlayFabGSDKGameInstance);

void UTestGameInstanceGSDK::Init()
{
	FOnGSDKShutdown_Dyn OnGsdkShutdown;
	OnGsdkShutdown.BindDynamic(this, &UTestGameInstanceGSDK::OnGSDKShutdown);
	FOnGSDKHealthCheck_Dyn OnGsdkHealthCheck;
	OnGsdkHealthCheck.BindDynamic(this, &UTestGameInstanceGSDK::OnGSDKHealthCheck);
	FOnGSDKServerActive_Dyn OnGSDKServerActive;
	OnGSDKServerActive.BindDynamic(this, &UTestGameInstanceGSDK::OnGSDKServerActive);
	FOnGSDKReadyForPlayers_Dyn OnGSDKReadyForPlayers;
	OnGSDKReadyForPlayers.BindDynamic(this, &UTestGameInstanceGSDK::OnGSDKReadyForPlayers);
	FOnGSDKMaintenanceV2_Dyn OnGSDKMaintenanceV2;
	OnGSDKMaintenanceV2.BindDynamic(this, &UTestGameInstanceGSDK::OnGSDKMaintenanceV2);

	UGSDKUtils::RegisterGSDKShutdownDelegate(OnGsdkShutdown);
	UGSDKUtils::RegisterGSDKHealthCheckDelegate(OnGsdkHealthCheck);
	UGSDKUtils::RegisterGSDKServerActiveDelegate(OnGSDKServerActive);
	UGSDKUtils::RegisterGSDKReadyForPlayers(OnGSDKReadyForPlayers);
	UGSDKUtils::RegisterGSDKMaintenanceV2Delegate(OnGSDKMaintenanceV2);

#if UE_SERVER
	UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Reached Init!"));
	UGSDKUtils::SetDefaultServerHostPort();
#endif
}

void UTestGameInstanceGSDK::OnStart()
{
	UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Reached onStart!"));
	UGSDKUtils::ReadyForPlayers();
}

void UTestGameInstanceGSDK::OnGSDKShutdown()
{
	UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Shutdown!"));
	FPlatformMisc::RequestExit(false);
}

bool UTestGameInstanceGSDK::OnGSDKHealthCheck()
{
	UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Healthy!"));
	return true;
}

void UTestGameInstanceGSDK::OnGSDKServerActive()
{
	/**
	 * Server is transitioning to an active state.
	 * Optional: Add in the implementation any code that is needed for the game server when
	 * this transition occurs.
	 */
	UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Active!"));
}

void UTestGameInstanceGSDK::OnGSDKReadyForPlayers()
{
	/**
	 * Server is transitioning to a StandBy state. Game initialization is complete and the game is ready
	 * to accept players.
	 * Optional: Add in the implementation any code that is needed for the game server before
	 * initialization completes.
	 */
	UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Finished Initialization - Moving to StandBy!"));
}

void UTestGameInstanceGSDK::OnGSDKMaintenanceV2(const FMaintenanceSchedule& schedule)
{
	/**
	* Server recieved a maintenance event.
	* Optional: Add in the implementation any code that is needed for the game server when
	* this transition occurs.
	*/
	UE_LOG(LogPlayFabGSDKGameInstance, Warning, TEXT("Recieved maintenance event: %s, %s, %s"), *schedule.Events[0].EventType, *schedule.Events[0].EventStatus, *schedule.Events[0].EventSource);
}

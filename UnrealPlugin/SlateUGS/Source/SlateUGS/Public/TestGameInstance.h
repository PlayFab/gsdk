// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TestGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayFabGSDKGameInstance, Log, All);

/**
 * 
 */
UCLASS()
class SLATEUGS_API UTestGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void OnStart() override;

protected:
    UFUNCTION()
    void OnGSDKShutdown();

    UFUNCTION()
    bool OnGSDKHealthCheck();

    UFUNCTION()
    void OnGSDKServerActive();

    UFUNCTION()
    void OnGSDKReadyForPlayers();
};

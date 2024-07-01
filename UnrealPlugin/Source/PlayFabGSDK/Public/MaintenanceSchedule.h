// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "MaintenanceSchedule.generated.h"

USTRUCT(BlueprintType)
struct FMaintenanceEvent
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FString EventId;
	FString EventType;
	FString ResourceType;
	TArray<FString> Resources;
	FString EventStatus;
	FDateTime NotBefore;
	FString Description;
	FString EventSource;
	uint32_t DurationInSeconds;
};

USTRUCT(BlueprintType)
struct FMaintenanceSchedule
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	FString DocumentIncarnation;
	TArray<FMaintenanceEvent> Events;
};

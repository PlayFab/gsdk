// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "ConnectedPlayer.generated.h"

USTRUCT(BlueprintType)
struct FConnectedPlayer
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	FString PlayerId;
};

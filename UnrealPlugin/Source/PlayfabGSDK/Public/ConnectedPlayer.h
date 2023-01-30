// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "ConnectedPlayer.generated.h"
FString PlayerId;
USTRUCT(BlueprintType)
struct FConnectedPlayer
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;
};

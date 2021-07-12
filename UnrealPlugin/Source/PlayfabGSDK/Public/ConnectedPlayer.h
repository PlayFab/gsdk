#pragma once

#include "CoreMinimal.h"

struct FConnectedPlayer
{
public:
	FString PlayerId;

	FConnectedPlayer(const FString& InPlayerId)
	{
		PlayerId = InPlayerId;
	}
};

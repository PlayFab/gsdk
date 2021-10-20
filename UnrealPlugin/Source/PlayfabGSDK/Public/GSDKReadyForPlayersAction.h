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

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GSDKReadyForPlayersAction.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEmptyDelegate);
/**
 * 
 */
UCLASS(MinimalAPI)
class UGSDKReadyForPlayersAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FEmptyDelegate OnReadyForPlayers;
	
	UPROPERTY(BlueprintAssignable)
	FEmptyDelegate OnTerminating;
	
	/// Called when the game server is ready to accept clients.
	/// This is a blocking call and will only return when this server is either allocated (a player is about to connect) or terminated.
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "PlayFab|GSDK|Players")
	static UGSDKReadyForPlayersAction* ReadyForPlayers();

	virtual void Activate() override;
};

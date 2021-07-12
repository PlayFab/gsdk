#pragma once

#include "CoreMinimal.h"

class FGSDKOutputDevice : public FOutputDevice
{
public:

	FGSDKOutputDevice();

	~FGSDKOutputDevice();

	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category) override;
};

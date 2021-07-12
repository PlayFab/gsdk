#include "GSDKOutputDevice.h"

#include "PlayfabGSDK.h"

FGSDKOutputDevice::FGSDKOutputDevice()
{
	check(GLog);
	GLog->AddOutputDevice(this);
}

FGSDKOutputDevice::~FGSDKOutputDevice()
{
	GLog->RemoveOutputDevice(this);
}

void FGSDKOutputDevice::Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category)
{
	const FString MessageToSend = FString::Printf(TEXT("%s: %s"), *Category.ToString(), Message);

	FPlayfabGSDKModule::Get().LogMessage(MessageToSend);
}

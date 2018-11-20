#include <gsdkCommonPch.h>

#include <playfab/PlayFabSettings.h>

namespace PlayFab
{
    const std::string PlayFabSettings::sdkVersion = "2.0.180420";
    const std::string PlayFabSettings::buildIdentifier = "xplatcppsdk_manual";
    const std::string PlayFabSettings::versionString = "XPlatCppSdk-2.0.180420";

    bool PlayFabSettings::useDevelopmentEnvironment = false;
    std::string PlayFabSettings::serverURL;
    std::string PlayFabSettings::developmentEnvironmentURL = ".playfabsandbox.com";
    std::string PlayFabSettings::productionEnvironmentURL = ".playfabapi.com";
    std::string PlayFabSettings::titleId; // You must set this value for PlayFabSdk to work properly (Found in the Game Manager for your title, at the PlayFab Website)
    ErrorCallback PlayFabSettings::globalErrorHandler = nullptr;

    // Control whether all callbacks are threaded or whether the user manually controlls callback timing from their main-thread
    bool PlayFabSettings::threadedCallbacks = false;

    std::string PlayFabSettings::entityToken; // This is set by entity GetEntityToken method, and is required by all other Entity API methods
#if defined(ENABLE_PLAYFABSERVER_API) || defined(ENABLE_PLAYFABADMIN_API)
    std::string PlayFabSettings::developerSecretKey; // You must set this value for PlayFabSdk to work properly (Found in the Game Manager for your title, at the PlayFab Website)
#endif

#ifndef DISABLE_PLAYFABCLIENT_API
    std::string PlayFabSettings::clientSessionTicket; // This is set by any Client Login method, and is required for all other Client API methods
    std::string PlayFabSettings::advertisingIdType = ""; // Set this to the appropriate AD_TYPE_X constant below
    std::string PlayFabSettings::advertisingIdValue = ""; // Set this to corresponding device value

    bool PlayFabSettings::disableAdvertising = false;
    const std::string PlayFabSettings::AD_TYPE_IDFA = "Idfa";
    const std::string PlayFabSettings::AD_TYPE_ANDROID_ID = "Adid";
#endif

    void PlayFabSettings::ForgetAllCredentials()
    {
        entityToken.clear();
        clientSessionTicket.clear();
    }

    std::string PlayFabSettings::GetUrl(const std::string& urlPath)
    {
        if (serverURL.length() == 0)
            serverURL = "https://" + titleId + (useDevelopmentEnvironment ? developmentEnvironmentURL : productionEnvironmentURL);
        return serverURL + urlPath;
    }
}

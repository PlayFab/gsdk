
// Copyright (C) Microsoft Corporation. All rights reserved.

#include <ctime>
#include <cstdio>
#include <string>
#include "gsdk.h"
#include <playfab/PlayFabClientApi.h>
#include <playfab/PlayFabClientDataModels.h>
#include <playfab/PlayFabSettings.h>

void inShutdown()
{
    printf("GSDK is shutting me down!!!\n");
    std::exit(0);
}

bool isHealthy()
{
    return false;
}

void maintenanceScheduled(tm t)
{
    time_t local = timegm(&t);
    double delta = difftime(local, time(NULL));
    printf("UTC:   %s\n", asctime(gmtime(&local)));
    printf("local: %s\n", asctime(localtime(&local)));
    printf("delta: %f\n", delta);
}

void OnPlayFabFail(const PlayFab::PlayFabError& error, void*)
{
    printf(("========== PlayFab call Failed: " + error.GenerateReport() + "\n").c_str());
}

void OnProfile(const PlayFab::ClientModels::GetPlayerProfileResult& result, void*)
{
    printf(("========== PlayFab Profile Success: " + result.PlayerProfile->DisplayName + "\n").c_str());
}

void OnLoginSuccess(const PlayFab::ClientModels::LoginResult& result, void*)
{
    printf(("========== PlayFab Login Success: " + result.PlayFabId + "\n").c_str());

    printf("========== Starting PlayFab GetProfile API call.\n");
    PlayFab::ClientModels::GetPlayerProfileRequest request;
    PlayFab::PlayFabClientAPI::GetPlayerProfile(request, OnProfile, OnPlayFabFail);
}

int main()
{
    printf("Starting!\n");
    try
    {
        // Super hacky short-term functionality PlayFab Test - TODO: Put the regular set of tests into proper Unit Test project
        printf("========== Starting PlayFab Login API call.\n");
        PlayFab::PlayFabSettings::titleId = "6195";
        PlayFab::PlayFabSettings::threadedCallbacks = true;
        PlayFab::ClientModels::LoginWithCustomIDRequest request;
        request.CustomId = "test_GSDK";
        request.CreateAccount = true;
        PlayFab::PlayFabClientAPI::LoginWithCustomID(request, OnLoginSuccess, OnPlayFabFail);

        Microsoft::Azure::Gaming::GSDK::start();
        Microsoft::Azure::Gaming::GSDK::registerShutdownCallback(&inShutdown);
        Microsoft::Azure::Gaming::GSDK::registerHealthCallback(&isHealthy);
        Microsoft::Azure::Gaming::GSDK::registerMaintenanceCallback(&maintenanceScheduled);

        // Test grabbing config
        std::string configJson = Microsoft::Azure::Gaming::GSDK::getConfigAsJson();
        printf(" Config before Active:\n%s\n", configJson.c_str());

        printf("\tStanding by!\n");
        if (Microsoft::Azure::Gaming::GSDK::readyForPlayers())
        {
            printf("Game on!\n");

            std::vector<Microsoft::Azure::Gaming::ConnectedPlayer> players;
            players.push_back(Microsoft::Azure::Gaming::ConnectedPlayer("player1"));
            players.push_back(Microsoft::Azure::Gaming::ConnectedPlayer("player2"));
            Microsoft::Azure::Gaming::GSDK::updateConnectedPlayers(players);

            configJson = Microsoft::Azure::Gaming::GSDK::getConfigAsJson();
            printf(" Config after Active:\n%s\n", configJson.c_str());

            printf("Logs directory is: %s\n", Microsoft::Azure::Gaming::GSDK::getLogsDirectory().c_str());
        }
        else 
        {
            printf("Not allocated. Server is being shut down.\n");
        }
    }
    catch (const std::exception &ex)
    {
        printf("Problem initializing GSDK: %s\n", ex.what());
    }

    printf("Press enter to exit the program.\n");
    getchar();

    return 0;
}

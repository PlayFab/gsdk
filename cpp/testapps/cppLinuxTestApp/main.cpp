
// Copyright (C) Microsoft Corporation. All rights reserved.

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include "gsdk.h"

void inShutdown()
{
    printf("GSDK is shutting me down!!!\n");
    // quick_exit, not exit: main() is blocked in getchar(), and exit()'s stdio cleanup would wait for it to release stdin.
    fflush(stdout);
    std::quick_exit(0);
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

int main()
{
    printf("Starting!\n");
    try
    {
        Microsoft::Azure::Gaming::GSDK::start();
        Microsoft::Azure::Gaming::GSDK::registerShutdownCallback(&inShutdown);
        Microsoft::Azure::Gaming::GSDK::registerHealthCallback(&isHealthy);
        Microsoft::Azure::Gaming::GSDK::registerMaintenanceCallback(&maintenanceScheduled);

        // Test grabbing config
        printf(" Config before Active.\n");
        for (auto config : Microsoft::Azure::Gaming::GSDK::getConfigSettings())
        {
            printf("%s: %s\n", config.first.c_str(), config.second.c_str());
        }

        printf("\tStanding by!\n");
        if (Microsoft::Azure::Gaming::GSDK::readyForPlayers())
        {
            printf("Game on!\n");

            std::vector<Microsoft::Azure::Gaming::ConnectedPlayer> players;
            players.push_back(Microsoft::Azure::Gaming::ConnectedPlayer("player1"));
            players.push_back(Microsoft::Azure::Gaming::ConnectedPlayer("player2"));
            Microsoft::Azure::Gaming::GSDK::updateConnectedPlayers(players);

            printf(" Config after Active.\n");
            for (auto config : Microsoft::Azure::Gaming::GSDK::getConfigSettings())
            {
                printf("%s: %s\n", config.first.c_str(), config.second.c_str());
            }

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

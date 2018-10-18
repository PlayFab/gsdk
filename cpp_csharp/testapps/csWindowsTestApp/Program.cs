using System;
using System.Collections.Generic;

using Microsoft.Playfab.Gaming.GSDK.CSharp;

namespace gameserverSDKConsoleApp
{
    class Program
    {
        static int healthState = 0;
        static bool getIsHealthy()
        {
            healthState++;
            return healthState % 2 == 0; // Flip back and forth so we can test both states
        }

        static void maintenanceScheduled(DateTimeOffset time)
        {
            Console.WriteLine($"Maintenance scheduled for: {time.ToLocalTime():dddd, MMMM d, yyyy HH:mm:ss}, which is this many seconds from now: {(time - DateTimeOffset.Now).TotalSeconds}");
        }

        static void Main(string[] args)
        {
            // Test startup
            GameserverSDK.Start();

            GameserverSDK.RegisterShutdownCallback(() => Console.WriteLine("Server is shutting me down!!!!!"));
            GameserverSDK.RegisterHealthCallback(getIsHealthy);
            GameserverSDK.RegisterMaintenanceCallback(maintenanceScheduled);

            // Test grabbing config
            Console.WriteLine("Config before Active.");
            foreach (var config in GameserverSDK.getConfigSettings())
            {
                Console.WriteLine($"{config.Key}: {config.Value}");
            }

            if (GameserverSDK.ReadyForPlayers())
            {
                List<ConnectedPlayer> players = new List<ConnectedPlayer>()
                {
                new ConnectedPlayer("player1"),
                new ConnectedPlayer("player2")
                };
                GameserverSDK.UpdateConnectedPlayers(players);

                Console.WriteLine("Config after Active.");
                foreach (var config in GameserverSDK.getConfigSettings())
                {
                    Console.WriteLine($"{config.Key}: {config.Value}");
                }

                // Print a rainbow
                int size = 13;
                for (int i = 0; i < 5; i++)
                {
                    String edge = new String(new char[size]).Replace('\0', ' ');
                    String middle = new String(new char[2 * i]).Replace('\0', ' ');
                    Console.WriteLine(edge + "====" + middle + "=====" + edge);
                    size--;
                }

                for (int i = 0; i < 4; i++)
                {
                    String edge = new String(new char[size]).Replace('\0', ' ');
                    String middle = new String(new char[10]).Replace('\0', ' ');
                    Console.WriteLine(edge + "====" + middle + "=====" + edge);
                }

                // Leave running a bit more to see the heartbeats
                for (int i = 0; i < 10; i++)
                {
                    Console.WriteLine("...");
                    System.Threading.Thread.Sleep(1000);
                }
            }
            else
            {
                Console.WriteLine("Did not activate, instead shutting down...");
            }

            Console.WriteLine("Press enter to exit.");
            Console.ReadKey();
        }
    }
}

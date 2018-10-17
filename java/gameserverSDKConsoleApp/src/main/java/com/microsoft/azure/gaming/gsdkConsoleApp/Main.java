package com.microsoft.azure.gaming.gsdkConsoleApp;

import com.microsoft.azure.gaming.*;

import java.time.ZoneId;
import java.util.ArrayList;
import java.util.Map;

/**
 * Simple app that lets us test the various Java GSDK methods
 *
 * Created by briantre on 6/15/2017.
 */
public class Main {
    static int state = 0;
    public static void main(String[] args){
        try {
            // Startup
            // GameserverSDK.start();
            String logsFolder = GameserverSDK.getLogsDirectory();
            System.out.println("The logs folder is: " + logsFolder);
            GameserverSDK.registerShutdownCallback(() -> System.out.println("I was just shutdown."));
            GameserverSDK.registerHealthCallback(() -> {
                state++;
                return state % 2 == 0? GameHostHealth.Healthy : GameHostHealth.Unhealthy;
            });
            GameserverSDK.registerMaintenanceCallback((maintenance) -> {
                System.out.println("There is a new scheduled maintenance happening at: "
                                    + maintenance.withZoneSameInstant(ZoneId.systemDefault()).toString());
            });
            GameserverSDK.readyForPlayers();

            ArrayList<ConnectedPlayer> players = new ArrayList<ConnectedPlayer>();
            players.add(new ConnectedPlayer("gamertag1"));
            GameserverSDK.updateConnectedPlayers(players);

            // Grab config
            Map<String, String> settings = GameserverSDK.getConfigSettings();
            System.out.println(settings.toString());

            // Print a rainbow
            int size = 13;
            for (int i = 0; i < 5; i++)
            {
                String edge = new String(new char[size]).replace('\0', ' ');
                String middle = new String(new char[2 * i]).replace('\0', ' ');
                System.out.println(edge + "====" + middle + "=====" + edge);
                size--;
            }

            for (int i = 0; i < 4; i++)
            {
                String edge = new String(new char[size]).replace('\0', ' ');
                String middle = new String(new char[10]).replace('\0', ' ');
                System.out.println(edge + "====" + middle + "=====" + edge);
            }

            players.add(new ConnectedPlayer("gamertag2"));
            GameserverSDK.updateConnectedPlayers(players);

            // Keep the game running to test GSDK
            for (int i = 0; i < 30; i++)
            {
                System.out.println("...");
                Thread.sleep(1000);
            }

            System.out.println("Ending");
        }
        catch(GameserverSDKInitializationException ex) {
            System.out.println(ex.toString());
        }
        catch (Exception e){
            System.out.println(e.toString());
        }
    }
}

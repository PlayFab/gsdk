package com.microsoft.azure.gaming;

import java.time.ZonedDateTime;
import java.util.*;
import java.util.concurrent.TimeUnit;
import java.util.function.*;

/**
 * The SDK game hosts can use to integrate with PlayFab Multiplayer Servers.
 */
public class GameserverSDK {
    /**
     * Key to use in the map returned by getConfigSettings to retrieve the agent url
     */
    public static final String HEARTBEAT_ENDPOINT_KEY = "heartbeatEndpoint";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the game server instance id
     */
    public static final String SERVER_ID_KEY = "serverId";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the log folder that is uploaded to Azure blob
     * storage once the container exits
     */
    public static final String LOG_FOLDER_KEY = "logFolder";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the folder which contains all game
     * certificate files
     */
    public static final String CERTIFICATE_FOLDER_KEY = "certificateFolder";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the PlayFab title Id for this game server
     */
    public static final String TITLE_ID_KEY = "titleId";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the PlayFab build Id for this game server
     */
    public static final String BUILD_ID_KEY = "buildId";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the region this game server is running in
     */
    public static final String REGION_KEY = "region";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the session cookie passed in to this
     * game server from the allocate call. Note that this key will only appear in the map after allocation,
     * once readyForPlayers returns true.
     */
    public static final String SESSION_COOKIE_KEY = "sessionCookie";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the session id for this game server, as
     * specified in the allocate call. Note that this key will only appear in the map after allocation, once
     * readyForPlayers returns true.
     */
    public static final String SESSION_ID_KEY = "sessionId";

    /**
     * Key to use in the map returned by getConfigSettings to retrieve the vm id of the virtual machine that
     * this game server is running on.
     */
    public static final String VM_ID_KEY = "vmId";

    private static boolean isInitialized = false;

    private static HeartbeatThread heartbeatThread;

    private static final Configuration gsdkConfiguration;

    private GameserverSDK(){
        // Users should never be able to instantiate this class.
    }

    // Calling start here so that any method users call will guarantee to have the GSDK initialized
    static {
        gsdkConfiguration = new JsonFileConfiguration();
        Logger.SetLogFolder(gsdkConfiguration.getLogFolder());
        start();
    }

    /**
     * Blocks indefinitely until the Agent tells us to activate or terminate
     * @throws GameserverSDKInitializationException if our thread is interrupted while waiting
     * @return True if the server has been allocated (is about to receive players). False if the server is terminated.
     */
    public static boolean readyForPlayers() throws GameserverSDKInitializationException {
        if (heartbeatThread.getState() != SessionHostStatus.Active)
        {
            heartbeatThread.setState(SessionHostStatus.StandingBy);

            // Wait for active semaphore
            try {
                heartbeatThread.waitForTransitionToActive(0, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt(); // Let caller know this thread is being interrupted
                throw new GameserverSDKInitializationException("Our thread was interrupted while waiting for Active.", e);
            }
        }

        return heartbeatThread.getState() == SessionHostStatus.Active;
    }

    /**
     * Retrieves a Map of config settings from the Agent
     * @return the configuration settings, note that these are
     * only guaranteed to be populated after the readyForPlayers()
     * call returns.
     */
    public static Map<String, String> getConfigSettings() {
        return heartbeatThread.getConfigSettings();
    }

    /**
     * Starts the heartbeat to the Agent.
     *
     * @throws GameserverSDKInitializationException if there was a problem initializing the GSDK
     */
    public static synchronized void start() {
        if (!isInitialized)
        {
            heartbeatThread = new HeartbeatThread(SessionHostStatus.Initializing, gsdkConfiguration);
            Thread heartbeat = new Thread(heartbeatThread);
            heartbeat.setDaemon(true);
            heartbeat.start();
            isInitialized = true;
        }
    }

    /**
     * Tells the agent which players are connected to this game host
     * @param players the updated list of players that are currently connected
     */
    public static void updateConnectedPlayers(List<ConnectedPlayer> players){
        heartbeatThread.setConnectedPlayers(players);
    }

    /**
     * Registers a function that will be called when the Agent tells us to Terminate
     * @param callback the function we will call on Terminate
     */
    public static void registerShutdownCallback(Runnable callback){
        heartbeatThread.registerShutdownCallback(callback);
    }

    /**
     * Registers a function that will be called when the Agent asks for our game health
     * @param callback the function we will call when we need to get the game host health
     */
    public static void registerHealthCallback(Supplier<GameHostHealth> callback){
        heartbeatThread.registerHealthCallback(callback);
    }

    /**
     * Registers a function that will be called when the Agent tells us a scheduled
     * maintenance is coming up, we will pass in the UTC Datetime of the maintenance event
     * @param callback the function we will call on a new scheduled maintenance
     */
    public static void registerMaintenanceCallback(Consumer<ZonedDateTime> callback){
        heartbeatThread.registerMaintenanceCallback(callback);
    }

    /**
     * Provides a way for the game host to log a custom message.
     * @param message the message to log
     */
    public static void log(String message) {
        Logger.Instance().Log(message);
    }

    /**
     * Returns a path to the directory where the game server can save any custom log files.
     * All files in this path will be zipped and uploaded to Azure Blob Storage once the container exits
     * @return path to the folder where all logs should be saved
     */
    public static String getLogsDirectory(){
        return gsdkConfiguration.getLogFolder();
    }

    /**
     * Returns a path to the directory shared by all game servers on the VM to store user generated content and other data.
     * @return path to the folder shared by all game servers on the VM.
     */
    public static String getSharedContentDirectory(){
        return gsdkConfiguration.getSharedContentFolder();
    }

    /**
     * Returns a path to the directory where game certificate files can be found
     * @return path to the certificate files
     */
    public static String getCertificateDirectory(){
        return gsdkConfiguration.getCertificateFolder();
    }

    /**
     * Returns the identity of the virtual machine that the games server is running on.
     * @return identity of the virtual machine.
     */
    public static String getVmId(){
        return gsdkConfiguration.getVmId();
    }

    /**
     * After allocation, returns a list of the initial players that have access to this game server,
     * used by PlayFab's Matchmaking offering
     * @return A list of player ids of the initial players that will connect
     */
    public static List<String> getInitialPlayers() { return heartbeatThread.getInitialPlayers(); }
}

package com.microsoft.azure.gaming;

import java.time.ZonedDateTime;
import java.util.*;
import java.util.concurrent.TimeUnit;
import java.util.function.*;

/**
 * The SDK game hosts can use to integrate with XCloud.
 *
 * Microsoft Copyright.
 */
public class GameserverSDK {
    // Keys for the map returned by getConfigSettings
    public static final String HEARTBEAT_ENDPOINT_KEY = "gsmsBaseUrl";
    public static final String SERVER_ID_KEY = "instanceId";
    public static final String LOG_FOLDER_KEY = "logFolder";
    public static final String CERTIFICATE_FOLDER_KEY = "certificateFolder";
    public static final String TITLE_ID_KEY = "titleId";
    public static final String BUILD_ID_KEY = "buildId";
    public static final String REGION_KEY = "region";

    // These two keys are only available in the map after allocation (once readyForPlayers returns true)
    public static final String SESSION_COOKIE_KEY = "sessionCookie";
    public static final String SESSION_ID_KEY = "sessionId";

    private static boolean isInitialized = false;

    private static HeartbeatThread heartbeatThread;

    private static final Configuration gsdkConfiguration;

    private GameserverSDK(){
        // Users should never be able to instantiate this class.
    }

    // Calling start here so that any method users call will guarantee to have the GSDK initialized
    static {
        // TODO: Get rid of this check once we fully migrate to JsonFileConfig
        if (System.getenv(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME) != null)
        {
            gsdkConfiguration = new JsonFileConfiguration();
        }
        else
        {
            gsdkConfiguration = new EnvironmentVariableConfiguration();
        }

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
     * Tells the agent how many players are connected to this game host
     * @param players
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

    public static String getLogsDirectory(){
        return gsdkConfiguration.getLogFolder();
    }

    /**
     * Returns a path to the directory where game certificate files can be found
     * @return path to the certificate files
     */
    public static String getCertificateDirectory(){
        return gsdkConfiguration.getCertificateFolder();
    }
}

package com.microsoft.azure.gaming;

import com.github.rholder.retry.*;
import com.google.gson.*;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.StatusLine;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpResponseException;
import org.apache.http.client.fluent.Request;
import org.apache.http.client.utils.URIBuilder;
import org.apache.http.entity.ContentType;

import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URI;
import java.net.URISyntaxException;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;
import java.util.function.Supplier;

/**
 * Class that handles sending a heartbeat to the Agent,
 * and dealing with the Agent's response. This is the meat
 * of the GSDK.
 */
class HeartbeatThread implements Runnable {
    private SessionHostStatus serverState;
    private List<ConnectedPlayer> connectedPlayers;
    private Map<String, String> configSettings;
    private int heartbeatInterval;
    private List<String> initialPlayers;
    private Runnable shutdownCallback;
    private Supplier<GameHostHealth> healthCallback;
    private Consumer<ZonedDateTime> maintenanceCallback;
    private Consumer<MaintenanceSchedule> maintenanceV2Callback;
    private ZonedDateTime lastScheduledMaintenanceUtc;
    private final Semaphore transitionToActive = new Semaphore(0);
    private final Semaphore signalHeartbeat = new Semaphore(0);
    private final URI agentUri;

    private final int MAXIMUM_NUM_HEARTBEAT_RETRIES = 8;
    private final int WAIT_TIME_BETWEEN_HEARTBEAT_RETRIES_MILLISECONDS = 1000;
    private final int HEARTBEAT_TIMEOUT_MILLISECONDS = 1000;

    /**
     * Set up the initial state and gather required info from environment variables.
     * The agent should have set up these env variables when it created the game host image.
     * @param initialState
     * @throws GameserverSDKInitializationException
     */
    protected HeartbeatThread(SessionHostStatus initialState, Configuration config) throws GameserverSDKInitializationException
    {
        Logger.Instance().Log("Initializing GSDK");

        this.setState(initialState);
        this.setConnectedPlayers(null);
        this.setInitialPlayers(null);
        this.lastScheduledMaintenanceUtc = null;
        this.transitionToActive.drainPermits();
        this.signalHeartbeat.drainPermits();

        config.validate();

        String agentEndpoint = config.getHeartbeatEndpoint();
        String serverId = config.getServerId();

        configSettings = Collections.synchronizedMap(new HashMap<String, String>());
        configSettings.putAll(config.getBuildMetadata());
        configSettings.putAll(config.getGamePorts());
        configSettings.put(GameserverSDK.HEARTBEAT_ENDPOINT_KEY, config.getHeartbeatEndpoint());
        configSettings.put(GameserverSDK.SERVER_ID_KEY, config.getServerId());
        configSettings.put(GameserverSDK.LOG_FOLDER_KEY, config.getLogFolder());
        configSettings.put(GameserverSDK.CERTIFICATE_FOLDER_KEY, config.getCertificateFolder());
        configSettings.put(GameserverSDK.TITLE_ID_KEY, config.getTitleId());
        configSettings.put(GameserverSDK.BUILD_ID_KEY, config.getBuildId());
        configSettings.put(GameserverSDK.REGION_KEY, config.getRegion());

        Logger.Instance().Log("VM Agent Endpoint: " + agentEndpoint);
        Logger.Instance().Log("Instance Id: " + serverId);

        try {
            URI infoUri = new URIBuilder()
                    .setScheme("http")
                    .setHost(agentEndpoint)
                    .setPath("/v1/metrics/" + serverId + "/gsdkinfo")
                    .build();
            
            Gson gson = new Gson();
            GSDKInfo info = new GSDKInfo();

            Request.Post(infoUri)
                    .bodyString(gson.toJson(info), ContentType.APPLICATION_JSON)
                    .connectTimeout(HEARTBEAT_TIMEOUT_MILLISECONDS)
                    .socketTimeout(HEARTBEAT_TIMEOUT_MILLISECONDS)
                    .execute().handleResponse(HeartbeatThread::handleInfoResponse);
        } catch (Exception e) {
            Logger.Instance().LogError("Failed to contact Agent when sending info.");
        }

        // Send an initial heartbeat here in the constructor so that we can fail
        // quickly if the Agent is unreachable.
        try {

            this.agentUri = new URIBuilder()
                    .setScheme("http")
                    .setHost(agentEndpoint)
                    .setPath("/v1/sessionHosts/" + serverId)
                    .build();

            SessionHostHeartbeatInfo heartbeatInfo = this.sendHeartbeat(this.getState());
            this.heartbeatInterval = heartbeatInfo.getNextHeartbeatIntervalMs();
            PerformOperation(heartbeatInfo.getOperation());
        } catch (URISyntaxException | ExecutionException | RetryException e) {
            GameserverSDKInitializationException initException =
                    new GameserverSDKInitializationException("Failed to contact Agent. GSDK failed to initialize.", e);
            Logger.Instance().LogError(initException);
            throw initException;
        }
        
        Logger.Instance().Log("GSDK Initialized");
    }

    protected synchronized void setState(SessionHostStatus state)
    {
        if (this.serverState != state)
        {
            this.serverState = state;
            this.signalHeartbeat.release();
        }
    }

    protected synchronized SessionHostStatus getState()
    {
        return this.serverState;
    }

    protected synchronized void setConnectedPlayers(List<ConnectedPlayer> players) {
        this.connectedPlayers = players;
    }

    protected synchronized List<ConnectedPlayer> getConnectedPlayers() { return this.connectedPlayers; }

    protected synchronized void setInitialPlayers(List<String> initPlayers) {
        this.initialPlayers = initPlayers;
    }

    protected synchronized List<String> getInitialPlayers() { return this.initialPlayers; }

    protected synchronized void registerShutdownCallback(Runnable callback)
    {
        this.shutdownCallback = callback;
    }

    protected synchronized void registerHealthCallback(Supplier<GameHostHealth> callback)
    {
        this.healthCallback = callback;
    }

    protected synchronized void registerMaintenanceCallback(Consumer<ZonedDateTime> callback)
    {
        this.maintenanceCallback = callback;
    }

    protected synchronized void registerMaintenanceV2Callback(Consumer<MaintenanceSchedule> callback)
    {
        this.maintenanceV2Callback = callback;
    }

    protected Map<String, String> getConfigSettings() {
        // This is used by GameserverSDK.java when a user wants a copy of the current config settings
        // Note that a copy constructor uses an iterator, and even with Synchronized Collections, we
        // need to place any iterators in a manual synchronized block
        // https://docs.oracle.com/javase/6/docs/api/java/util/Collections.html#synchronizedSet%28java.util.Set%29
        synchronized (this.configSettings) {
            return new HashMap(this.configSettings);
        }
    }

    /**
     * Waits until the active semaphore has been signaled
     * @param timeout the ammount of time to wait (if <= 0 we wait indefinitely)
     * @param timeUnit the unit of time to wait
     * @return true if the semaphore was signaled, false if instead we timed out
     * @throws InterruptedException if the owner of the thread is asking us to stop
     */
    public boolean waitForTransitionToActive(long timeout, TimeUnit timeUnit) throws InterruptedException
    {
        if (timeout <= 0) {
            transitionToActive.acquire();
            return true;
        } else {
            return transitionToActive.tryAcquire(timeout, timeUnit);
        }
    }

    /**
     * This method heartbeats to the agent every second until
     * we receive a Terminate operation
     */
    @Override
    public void run() {
        while (true) {
            try {
                // Wait until our interval times out or a state change happens
                if (signalHeartbeat.tryAcquire(this.heartbeatInterval, TimeUnit.MILLISECONDS))
                {
                    Logger.Instance().Log("State transition signaled an early heartbeat.");
                    signalHeartbeat.drainPermits(); // Ensure we need a new signal to break in early again
                }

                // Send our heartbeat
                SessionHostStatus currentState = this.getState();
                SessionHostHeartbeatInfo sessionInfo = sendHeartbeat(currentState);
                this.heartbeatInterval = sessionInfo.getNextHeartbeatIntervalMs();

                // If they sent us a config, save it
                SessionConfig responseConfig = sessionInfo.getSessionConfig();
                if (responseConfig != null)
                {
                    configSettings.putAll(responseConfig.ToMapAllStrings());

                    if (responseConfig.getInitialPlayers() != null && !responseConfig.getInitialPlayers().isEmpty())
                    {
                        this.setInitialPlayers(responseConfig.getInitialPlayers());
                    }
                }

                // If there's a scheduled maintenance that we haven't notified about, do so now
                Consumer<ZonedDateTime> callback = this.maintenanceCallback;
                ZonedDateTime nextMaintenance = sessionInfo.getNextScheduledMaintenanceUtc();
                if (callback != null && nextMaintenance != null && !nextMaintenance.equals(this.lastScheduledMaintenanceUtc))
                {
                    callback.accept(nextMaintenance); // should this be in a new thread?
                    this.lastScheduledMaintenanceUtc = nextMaintenance; // cache it, since we only want to notify once
                }

                Consumer<MaintenanceSchedule> maintV2Callback = this.maintenanceV2Callback;
                MaintenanceSchedule schedule = sessionInfo.getMaintenanceSchedule();

                if (maintV2Callback != null && schedule != null)
                {
                    maintV2Callback.accept(schedule);
                }

                // If Terminating, send a last heartbeat that we're done and exit the loop
                if (currentState == SessionHostStatus.Terminating)
                {
                    // Further shutdown logic can go here
                    sendHeartbeat(SessionHostStatus.Terminated);
                    break;
                }

                // Perform the operation that the server requested
                PerformOperation(sessionInfo.getOperation());
            } catch (RetryException e) {
                Logger.Instance().LogError("Exhausted all retry attempts when trying to send heartbeat to the agent.", e);
            } catch (Exception e) {
                Logger.Instance().LogError("Unexpected exception occurred when sending a heartbeat to the agent.  Terminating process.", e);
                Runnable temp = shutdownCallback;
                if (temp != null) {
                    temp.run();
                }
                System.exit(1);
            }
        }
    }

    /**
     * Uses Apache HttpClient to send the heartbeat to the agent.  This method will automaticall retry if a failure occurs.
     * @param currentState The state we're sending
     * @return The response we got from the Agent
     * @throws URISyntaxException
     * @throws RetryException If every attempt to send the heartbeat fails, then a RetryException is thrown.
     * @throws ExecutionException If an unexpected exception is raised by one of the retry attempts then that exception will
     * be included as the 'cause' in an ExceutionException.
     */
    private SessionHostHeartbeatInfo sendHeartbeat(SessionHostStatus currentState) throws RetryException, ExecutionException {

        Gson gson = new Gson();
        Retryer<SessionHostHeartbeatInfo> retryer = RetryerBuilder.<SessionHostHeartbeatInfo>newBuilder()
                .retryIfExceptionOfType(IOException.class)
                .retryIfExceptionOfType(HttpResponseException.class)
                .retryIfRuntimeException()
                .withStopStrategy(StopStrategies.stopAfterAttempt(MAXIMUM_NUM_HEARTBEAT_RETRIES))
                .withWaitStrategy(WaitStrategies.incrementingWait(WAIT_TIME_BETWEEN_HEARTBEAT_RETRIES_MILLISECONDS, TimeUnit.MILLISECONDS, 0, TimeUnit.MILLISECONDS))
                .build();

        SessionHostHeartbeatInfo result = retryer.call(() -> {
            SessionHostHeartbeatInfo heartbeatInfo = new SessionHostHeartbeatInfo();
            heartbeatInfo.setCurrentGameState(currentState);
            heartbeatInfo.setConnectedPlayers(this.getConnectedPlayers());

            Supplier<GameHostHealth> callback = this.healthCallback;
            if (callback != null)
            {
                heartbeatInfo.setCurrentGameHealth(callback.get());
            }

            heartbeatInfo = Request.Patch(agentUri)
                    .bodyString(gson.toJson(heartbeatInfo), ContentType.APPLICATION_JSON)
                    .connectTimeout(HEARTBEAT_TIMEOUT_MILLISECONDS)
                    .socketTimeout(HEARTBEAT_TIMEOUT_MILLISECONDS)
                    .execute().handleResponse(HeartbeatThread::handleHttpResponse);

            return heartbeatInfo;
        });

        StringBuilder playersString = new StringBuilder();
        if (this.getConnectedPlayers() != null && this.getConnectedPlayers().size() > 0) {
            this.getConnectedPlayers().forEach((p) -> playersString.append(p.getPlayerId() + ","));
        }

        Logger.Instance().Log("Heartbeat request: { state = " + currentState
                    + ", connectedPlayers = " + playersString
                    + " } response: { Next Operation = " + result.getOperation()
                    + " New Interval = " + result.getNextHeartbeatIntervalMs() + "}");
        return result;
    }

    private static SessionHostHeartbeatInfo handleHttpResponse(HttpResponse response) throws IOException {
        // Create a gson object that knows how to handle LocalDateTime
        Gson gson = new GsonBuilder().registerTypeAdapter(ZonedDateTime.class, (JsonDeserializer<ZonedDateTime>) (json, type, jsonDeserializationContext) ->
                        ZonedDateTime.parse(json.getAsJsonPrimitive().getAsString()).withZoneSameLocal(ZoneId.of("UTC"))).create();
        
        StatusLine statusLine = response.getStatusLine();
        HttpEntity entity = response.getEntity();
        if (statusLine.getStatusCode() >= 300) {
            Logger.Instance().LogWarning("Received non-success code from Agent.  Status Code: " + statusLine.getStatusCode() + ".  Reason: " + statusLine.getReasonPhrase());
            throw new HttpResponseException(
                    statusLine.getStatusCode(),
                    statusLine.getReasonPhrase());
        }
        
        if (entity == null) {
            final String warningMessage = "Agent provided a non-failure resonse code, but did not include any content.";
            Logger.Instance().LogWarning(warningMessage);
            throw new ClientProtocolException(warningMessage);
        }

        InputStreamReader reader = new InputStreamReader(entity.getContent());
        return gson.fromJson(reader, SessionHostHeartbeatInfo.class);
    }

    private static Void handleInfoResponse(HttpResponse response) throws IOException {
        StatusLine statusLine = response.getStatusLine();
        if (statusLine.getStatusCode() >= 300) {
            Logger.Instance().LogWarning("Received non-success code from Agent.  Status Code: " + statusLine.getStatusCode() + ".  Reason: " + statusLine.getReasonPhrase());
            throw new HttpResponseException(
                    statusLine.getStatusCode(),
                    statusLine.getReasonPhrase());
        }

        return null;
    }

    /**
     * Handles the agent response Operation
     * @param newOperation the operation we received
     */
    private void PerformOperation(Operation newOperation)
    {
        switch(newOperation) {
            case CONTINUE:
                // No action required
                break;
            case ACTIVE:
                if (this.getState() != SessionHostStatus.Active) {
                    this.setState(SessionHostStatus.Active);
                    this.transitionToActive.release();
                }
                break;
            case TERMINATE:
                if (this.getState() != SessionHostStatus.Terminating) {
                    this.setState(SessionHostStatus.Terminating);
                    this.transitionToActive.release();
                    Runnable temp = shutdownCallback;
                    if (temp != null) {
                        temp.run();
                    }
                }
                break;
            default:
                Logger.Instance().LogError("Unknown operation received: " + newOperation);
        }

    }
}

package com.microsoft.azure.gaming;

import java.time.ZonedDateTime;
import java.util.List;

/**
 * Encapsulates the Heartbeat response we receive from the Agent.
 *
 * Created by hamonciv on 7/17/2017.
 */
public class SessionHostHeartbeatInfo {
    /**
     * The current game state. For example - StandingBy, Active, etc.
     */
    private SessionHostStatus currentGameState;

    /**
     * The last queried game host health status
     */
    private GameHostHealth currentGameHealth;

    /**
     * Keeps track of the current list of connected players
     */
    private List<ConnectedPlayer> currentPlayers;

    /**
     * The number of milliseconds to wait before sending the next heartbeat.
     */
    private Integer nextHeartbeatIntervalMs;

    /**
     * The next operation the VM Agent wants us to take
     */
    private Operation operation;

    /**
     * The configuration sent down to the game host from Control Plane
     */
    private SessionConfig sessionConfig;

    /**
     * The next scheduled maintenance time from Azure, in UTC
     */
    private ZonedDateTime nextScheduledMaintenanceUtc;

    public SessionHostStatus getCurrentGameState() { return currentGameState; }

    public void setCurrentGameState(SessionHostStatus newSessionHostStatus)
    {
        this.currentGameState = newSessionHostStatus;
    }

    public Integer getNextHeartbeatIntervalMs()
    {
        return nextHeartbeatIntervalMs;
    }

    public void setNextHeartbeatIntervalMs(Integer newHeartbeatIntervalMs)
    {
        this.nextHeartbeatIntervalMs = newHeartbeatIntervalMs;
    }

    public Operation getOperation() {
        return operation;
    }

    public void setOperation(Operation newOperation) {
        this.operation = newOperation;
    }


    public GameHostHealth getCurrentGameHealth() { return currentGameHealth; }

    public void setCurrentGameHealth(GameHostHealth currentGameHealth) {
        this.currentGameHealth = currentGameHealth;
    }

    public List<ConnectedPlayer> getCurrentPlayers() { return this.currentPlayers; }

    public void setConnectedPlayers(List<ConnectedPlayer> players) {
        this.currentPlayers = players;
    }

    public SessionConfig getSessionConfig() {
        return sessionConfig;
    }

    public void setSessionConfig(SessionConfig sessionConfig) {
        this.sessionConfig = sessionConfig;
    }

    public ZonedDateTime getNextScheduledMaintenanceUtc() { return nextScheduledMaintenanceUtc; }

    public void setNextScheduledMaintenanceUtc(ZonedDateTime nextScheduledMaintenanceUtc)
    {
        this.nextScheduledMaintenanceUtc = nextScheduledMaintenanceUtc;
    }

}

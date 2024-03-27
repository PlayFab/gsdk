package com.microsoft.azure.gaming;

import java.time.ZonedDateTime;
import java.util.List;

/**
 * Encapsulates the Heartbeat response we receive from the Agent.
 */
class SessionHostHeartbeatInfo {
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

    /**
     * Schedule of upcoming maintenance events from Azure
     */
    private MaintenanceSchedule maintenanceSchedule;

    protected SessionHostStatus getCurrentGameState() { return currentGameState; }

    protected void setCurrentGameState(SessionHostStatus newSessionHostStatus)
    {
        this.currentGameState = newSessionHostStatus;
    }

    protected Integer getNextHeartbeatIntervalMs()
    {
        return nextHeartbeatIntervalMs;
    }

    protected void setNextHeartbeatIntervalMs(Integer newHeartbeatIntervalMs)
    {
        this.nextHeartbeatIntervalMs = newHeartbeatIntervalMs;
    }

    protected Operation getOperation() {
        return operation;
    }

    protected void setOperation(Operation newOperation) {
        this.operation = newOperation;
    }


    protected GameHostHealth getCurrentGameHealth() { return currentGameHealth; }

    protected void setCurrentGameHealth(GameHostHealth currentGameHealth) {
        this.currentGameHealth = currentGameHealth;
    }

    protected List<ConnectedPlayer> getCurrentPlayers() { return this.currentPlayers; }

    protected void setConnectedPlayers(List<ConnectedPlayer> players) {
        this.currentPlayers = players;
    }

    protected SessionConfig getSessionConfig() {
        return sessionConfig;
    }

    protected void setSessionConfig(SessionConfig sessionConfig) {
        this.sessionConfig = sessionConfig;
    }

    protected ZonedDateTime getNextScheduledMaintenanceUtc() { return nextScheduledMaintenanceUtc; }
    
    protected void setNextScheduledMaintenanceUtc(ZonedDateTime nextScheduledMaintenanceUtc)
    {
        this.nextScheduledMaintenanceUtc = nextScheduledMaintenanceUtc;
    }

    protected MaintenanceSchedule getMaintenanceSchedule() { return maintenanceSchedule; }

    protected void setMaintenanceSchedule(MaintenanceSchedule maintenanceSchedule)
    {
        this.maintenanceSchedule = maintenanceSchedule;
    }

}

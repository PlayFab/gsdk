package com.microsoft.azure.gaming;

/**
 * Describes a player that is connected to the game server.
 */
public class ConnectedPlayer {
    private String playerId;

    public ConnectedPlayer(String newPlayerId){
        this.setPlayerId(newPlayerId);
    }

    public String getPlayerId() {
        return playerId;
    }

    public void setPlayerId(String playerId) {
        this.playerId = playerId;
    }

}

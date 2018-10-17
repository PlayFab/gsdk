package com.microsoft.azure.gaming;

/**
 * Created by briantre on 6/16/2017.
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

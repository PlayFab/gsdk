package com.microsoft.azure.gaming;

import org.junit.Test;
import static org.junit.Assert.*;

public class ConnectedPlayerTest {

    @Test
    public void constructor_setsPlayerId() {
        ConnectedPlayer player = new ConnectedPlayer("player1");
        assertEquals("player1", player.getPlayerId());
    }

    @Test
    public void setPlayerId_updatesPlayerId() {
        ConnectedPlayer player = new ConnectedPlayer("player1");
        player.setPlayerId("player2");
        assertEquals("player2", player.getPlayerId());
    }

    @Test
    public void constructor_handlesEmptyString() {
        ConnectedPlayer player = new ConnectedPlayer("");
        assertEquals("", player.getPlayerId());
    }

    @Test
    public void constructor_handlesNullPlayerId() {
        ConnectedPlayer player = new ConnectedPlayer(null);
        assertNull(player.getPlayerId());
    }
}

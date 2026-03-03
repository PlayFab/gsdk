package com.microsoft.azure.gaming;

import com.google.gson.Gson;
import org.junit.Test;

import java.util.Arrays;
import java.util.Map;
import java.util.UUID;

import static org.junit.Assert.*;

public class SessionConfigTest {

    @Test
    public void gettersAndSetters_sessionId() {
        SessionConfig config = new SessionConfig();
        UUID id = UUID.randomUUID();
        config.setSessionId(id);
        assertEquals(id, config.getSessionId());
    }

    @Test
    public void gettersAndSetters_sessionCookie() {
        SessionConfig config = new SessionConfig();
        config.setSessionCookie("testCookie");
        assertEquals("testCookie", config.getSessionCookie());
    }

    @Test
    public void gettersAndSetters_initialPlayers() {
        SessionConfig config = new SessionConfig();
        config.setInitialPlayers(Arrays.asList("player1", "player2", "player3"));
        assertEquals(3, config.getInitialPlayers().size());
        assertEquals("player1", config.getInitialPlayers().get(0));
        assertEquals("player2", config.getInitialPlayers().get(1));
        assertEquals("player3", config.getInitialPlayers().get(2));
    }

    @Test
    public void toMapAllStrings_containsSessionIdAndCookie() {
        SessionConfig config = new SessionConfig();
        UUID id = UUID.fromString("a1b2c3d4-e5f6-7890-abcd-ef1234567890");
        config.setSessionId(id);
        config.setSessionCookie("awesomeCookie");

        Map<String, String> map = config.ToMapAllStrings();

        assertEquals("a1b2c3d4-e5f6-7890-abcd-ef1234567890", map.get(GameserverSDK.SESSION_ID_KEY));
        assertEquals("awesomeCookie", map.get(GameserverSDK.SESSION_COOKIE_KEY));
    }

    @Test
    public void toMapAllStrings_hasTwoEntries() {
        SessionConfig config = new SessionConfig();
        config.setSessionId(UUID.randomUUID());
        config.setSessionCookie("cookie");

        Map<String, String> map = config.ToMapAllStrings();

        assertEquals(2, map.size());
        assertTrue(map.containsKey(GameserverSDK.SESSION_ID_KEY));
        assertTrue(map.containsKey(GameserverSDK.SESSION_COOKIE_KEY));
    }

    @Test
    public void deserialization_fromJson_parsesCorrectly() {
        String json = "{"
                + "\"sessionId\":\"a1b2c3d4-e5f6-7890-abcd-ef1234567890\","
                + "\"sessionCookie\":\"testCookie\","
                + "\"initialPlayers\":[\"player1\",\"player2\"]"
                + "}";

        Gson gson = new Gson();
        SessionConfig config = gson.fromJson(json, SessionConfig.class);

        assertEquals(UUID.fromString("a1b2c3d4-e5f6-7890-abcd-ef1234567890"), config.getSessionId());
        assertEquals("testCookie", config.getSessionCookie());
        assertEquals(2, config.getInitialPlayers().size());
        assertEquals("player1", config.getInitialPlayers().get(0));
    }

    @Test
    public void deserialization_noInitialPlayers_returnsNull() {
        String json = "{"
                + "\"sessionId\":\"a1b2c3d4-e5f6-7890-abcd-ef1234567890\","
                + "\"sessionCookie\":\"testCookie\""
                + "}";

        Gson gson = new Gson();
        SessionConfig config = gson.fromJson(json, SessionConfig.class);

        assertNull(config.getInitialPlayers());
    }
}

package com.microsoft.azure.gaming;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * Configuration for a game session
 */
class SessionConfig {
    private UUID sessionId;

    private String sessionCookie;

    private List<String> initialPlayers;

    protected UUID getSessionId() {
        return sessionId;
    }

    public void setSessionId(UUID sessionId) {
        this.sessionId = sessionId;
    }


    protected String getSessionCookie() {
        return sessionCookie;
    }

    public void setSessionCookie(String sessionCookie) {
        this.sessionCookie = sessionCookie;
    }


    protected List<String> getInitialPlayers() {
        return initialPlayers;
    }

    public void setInitialPlayers(List<String> initialPlayers) {
        this.initialPlayers = initialPlayers;
    }

    protected Map<String, String> ToMapAllStrings()
    {
        HashMap<String, String> properties = new HashMap<String, String>();
        properties.put(GameserverSDK.SESSION_ID_KEY, this.getSessionId().toString());
        properties.put(GameserverSDK.SESSION_COOKIE_KEY, this.getSessionCookie());
        return properties;
    }
}

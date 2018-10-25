package com.microsoft.azure.gaming;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * Configuration for a game session
 */
class SessionConfig {
    private UUID sessionId;

    private String sessionCookie;

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

    protected Map<String, String> ToMap()
    {
        HashMap<String, String> properties = new HashMap<String, String>();
        properties.put("sessionId", this.getSessionId().toString());
        properties.put("sessionCookie", this.getSessionCookie());
        return properties;
    }
}

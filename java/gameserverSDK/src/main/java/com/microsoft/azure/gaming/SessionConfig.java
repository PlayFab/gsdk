package com.microsoft.azure.gaming;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * Created by hamonciv on 7/28/2017.
 */
public class SessionConfig {
    private UUID sessionId;

    private String sessionCookie;

    public UUID getSessionId() {
        return sessionId;
    }

    public void setSessionId(UUID sessionId) {
        this.sessionId = sessionId;
    }


    public String getSessionCookie() {
        return sessionCookie;
    }

    public void setSessionCookie(String sessionCookie) {
        this.sessionCookie = sessionCookie;
    }

    public Map<String, String> ToMap()
    {
        HashMap<String, String> properties = new HashMap<String, String>();
        properties.put("sessionId", this.getSessionId().toString());
        properties.put("sessionCookie", this.getSessionCookie());
        return properties;
    }
}

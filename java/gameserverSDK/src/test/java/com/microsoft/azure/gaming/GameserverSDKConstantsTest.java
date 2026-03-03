package com.microsoft.azure.gaming;

import org.junit.Test;

import static org.junit.Assert.*;

/**
 * Tests for GameserverSDK constant key values.
 * These constants are part of the public API and must remain stable.
 */
public class GameserverSDKConstantsTest {

    @Test
    public void heartbeatEndpointKey_hasExpectedValue() {
        assertEquals("heartbeatEndpoint", GameserverSDK.HEARTBEAT_ENDPOINT_KEY);
    }

    @Test
    public void serverIdKey_hasExpectedValue() {
        assertEquals("serverId", GameserverSDK.SERVER_ID_KEY);
    }

    @Test
    public void logFolderKey_hasExpectedValue() {
        assertEquals("logFolder", GameserverSDK.LOG_FOLDER_KEY);
    }

    @Test
    public void certificateFolderKey_hasExpectedValue() {
        assertEquals("certificateFolder", GameserverSDK.CERTIFICATE_FOLDER_KEY);
    }

    @Test
    public void titleIdKey_hasExpectedValue() {
        assertEquals("titleId", GameserverSDK.TITLE_ID_KEY);
    }

    @Test
    public void buildIdKey_hasExpectedValue() {
        assertEquals("buildId", GameserverSDK.BUILD_ID_KEY);
    }

    @Test
    public void regionKey_hasExpectedValue() {
        assertEquals("region", GameserverSDK.REGION_KEY);
    }

    @Test
    public void sessionCookieKey_hasExpectedValue() {
        assertEquals("sessionCookie", GameserverSDK.SESSION_COOKIE_KEY);
    }

    @Test
    public void sessionIdKey_hasExpectedValue() {
        assertEquals("sessionId", GameserverSDK.SESSION_ID_KEY);
    }

    @Test
    public void vmIdKey_hasExpectedValue() {
        assertEquals("vmId", GameserverSDK.VM_ID_KEY);
    }
}

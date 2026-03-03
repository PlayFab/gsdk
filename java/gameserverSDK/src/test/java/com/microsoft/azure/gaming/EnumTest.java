package com.microsoft.azure.gaming;

import com.google.gson.Gson;
import org.junit.Test;

import static org.junit.Assert.*;

/**
 * Tests for GameHostHealth and SessionHostStatus enums.
 */
public class EnumTest {

    @Test
    public void gameHostHealth_hasExpectedValues() {
        GameHostHealth[] values = GameHostHealth.values();
        assertEquals(2, values.length);
        assertEquals(GameHostHealth.Healthy, GameHostHealth.valueOf("Healthy"));
        assertEquals(GameHostHealth.Unhealthy, GameHostHealth.valueOf("Unhealthy"));
    }

    @Test
    public void sessionHostStatus_hasExpectedValues() {
        SessionHostStatus[] values = SessionHostStatus.values();
        assertEquals(7, values.length);
        assertNotNull(SessionHostStatus.valueOf("Invalid"));
        assertNotNull(SessionHostStatus.valueOf("Initializing"));
        assertNotNull(SessionHostStatus.valueOf("StandingBy"));
        assertNotNull(SessionHostStatus.valueOf("Active"));
        assertNotNull(SessionHostStatus.valueOf("Terminating"));
        assertNotNull(SessionHostStatus.valueOf("Terminated"));
        assertNotNull(SessionHostStatus.valueOf("Quarantined"));
    }

    @Test
    public void gameHostHealth_gsonSerialization() {
        Gson gson = new Gson();
        assertEquals("\"Healthy\"", gson.toJson(GameHostHealth.Healthy));
        assertEquals("\"Unhealthy\"", gson.toJson(GameHostHealth.Unhealthy));
    }

    @Test
    public void gameHostHealth_gsonDeserialization() {
        Gson gson = new Gson();
        assertEquals(GameHostHealth.Healthy, gson.fromJson("\"Healthy\"", GameHostHealth.class));
        assertEquals(GameHostHealth.Unhealthy, gson.fromJson("\"Unhealthy\"", GameHostHealth.class));
    }

    @Test
    public void sessionHostStatus_gsonSerialization() {
        Gson gson = new Gson();
        assertEquals("\"Initializing\"", gson.toJson(SessionHostStatus.Initializing));
        assertEquals("\"StandingBy\"", gson.toJson(SessionHostStatus.StandingBy));
        assertEquals("\"Active\"", gson.toJson(SessionHostStatus.Active));
    }
}

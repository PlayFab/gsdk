package com.microsoft.azure.gaming;

import com.google.gson.*;
import org.junit.Test;

import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.Arrays;
import java.util.List;

import static org.junit.Assert.*;

public class SessionHostHeartbeatInfoTest {

    @Test
    public void gettersAndSetters_currentGameState() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        info.setCurrentGameState(SessionHostStatus.Active);
        assertEquals(SessionHostStatus.Active, info.getCurrentGameState());
    }

    @Test
    public void gettersAndSetters_currentGameHealth() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        info.setCurrentGameHealth(GameHostHealth.Healthy);
        assertEquals(GameHostHealth.Healthy, info.getCurrentGameHealth());
    }

    @Test
    public void gettersAndSetters_nextHeartbeatIntervalMs() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        info.setNextHeartbeatIntervalMs(5000);
        assertEquals(Integer.valueOf(5000), info.getNextHeartbeatIntervalMs());
    }

    @Test
    public void gettersAndSetters_operation() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        info.setOperation(Operation.ACTIVE);
        assertEquals(Operation.ACTIVE, info.getOperation());
    }

    @Test
    public void gettersAndSetters_connectedPlayers() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        List<ConnectedPlayer> players = Arrays.asList(
                new ConnectedPlayer("player1"),
                new ConnectedPlayer("player2")
        );
        info.setConnectedPlayers(players);
        assertEquals(2, info.getCurrentPlayers().size());
        assertEquals("player1", info.getCurrentPlayers().get(0).getPlayerId());
        assertEquals("player2", info.getCurrentPlayers().get(1).getPlayerId());
    }

    @Test
    public void gettersAndSetters_sessionConfig() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        SessionConfig config = new SessionConfig();
        config.setSessionCookie("testCookie");
        info.setSessionConfig(config);
        assertEquals("testCookie", info.getSessionConfig().getSessionCookie());
    }

    @Test
    public void gettersAndSetters_nextScheduledMaintenanceUtc() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        ZonedDateTime maintTime = ZonedDateTime.of(2024, 1, 15, 10, 30, 0, 0, ZoneId.of("UTC"));
        info.setNextScheduledMaintenanceUtc(maintTime);
        assertEquals(maintTime, info.getNextScheduledMaintenanceUtc());
    }

    @Test
    public void gettersAndSetters_maintenanceSchedule() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        MaintenanceSchedule schedule = new MaintenanceSchedule();
        info.setMaintenanceSchedule(schedule);
        assertNotNull(info.getMaintenanceSchedule());
    }

    @Test
    public void deserialization_heartbeatResponse_parsesCorrectly() {
        String json = "{"
                + "\"currentGameState\":\"StandingBy\","
                + "\"nextHeartbeatIntervalMs\":1000,"
                + "\"operation\":\"Continue\","
                + "\"sessionConfig\":{"
                + "  \"sessionId\":\"a1b2c3d4-e5f6-7890-abcd-ef1234567890\","
                + "  \"sessionCookie\":\"testCookie\""
                + "}"
                + "}";

        Gson gson = new GsonBuilder().registerTypeAdapter(ZonedDateTime.class, (JsonDeserializer<ZonedDateTime>) (jsonEl, type, ctx) ->
                ZonedDateTime.parse(jsonEl.getAsJsonPrimitive().getAsString()).withZoneSameLocal(ZoneId.of("UTC"))).create();

        SessionHostHeartbeatInfo info = gson.fromJson(json, SessionHostHeartbeatInfo.class);

        assertEquals(Integer.valueOf(1000), info.getNextHeartbeatIntervalMs());
        assertEquals(Operation.CONTINUE, info.getOperation());
        assertNotNull(info.getSessionConfig());
        assertEquals("testCookie", info.getSessionConfig().getSessionCookie());
    }

    @Test
    public void deserialization_withNextScheduledMaintenance_parsesDateTime() {
        String json = "{"
                + "\"nextHeartbeatIntervalMs\":1000,"
                + "\"operation\":\"Continue\","
                + "\"nextScheduledMaintenanceUtc\":\"2024-01-15T10:30:00Z\""
                + "}";

        Gson gson = new GsonBuilder().registerTypeAdapter(ZonedDateTime.class, (JsonDeserializer<ZonedDateTime>) (jsonEl, type, ctx) ->
                ZonedDateTime.parse(jsonEl.getAsJsonPrimitive().getAsString()).withZoneSameLocal(ZoneId.of("UTC"))).create();

        SessionHostHeartbeatInfo info = gson.fromJson(json, SessionHostHeartbeatInfo.class);

        assertNotNull(info.getNextScheduledMaintenanceUtc());
        assertEquals(2024, info.getNextScheduledMaintenanceUtc().getYear());
        assertEquals(1, info.getNextScheduledMaintenanceUtc().getMonthValue());
        assertEquals(15, info.getNextScheduledMaintenanceUtc().getDayOfMonth());
        assertEquals(10, info.getNextScheduledMaintenanceUtc().getHour());
        assertEquals(30, info.getNextScheduledMaintenanceUtc().getMinute());
    }

    @Test
    public void deserialization_withActiveOperation_parsesCorrectly() {
        String json = "{"
                + "\"nextHeartbeatIntervalMs\":2000,"
                + "\"operation\":\"Active\""
                + "}";

        Gson gson = new Gson();
        SessionHostHeartbeatInfo info = gson.fromJson(json, SessionHostHeartbeatInfo.class);

        assertEquals(Operation.ACTIVE, info.getOperation());
        assertEquals(Integer.valueOf(2000), info.getNextHeartbeatIntervalMs());
    }

    @Test
    public void deserialization_withTerminateOperation_parsesCorrectly() {
        String json = "{"
                + "\"nextHeartbeatIntervalMs\":500,"
                + "\"operation\":\"Terminate\""
                + "}";

        Gson gson = new Gson();
        SessionHostHeartbeatInfo info = gson.fromJson(json, SessionHostHeartbeatInfo.class);

        assertEquals(Operation.TERMINATE, info.getOperation());
    }

    @Test
    public void deserialization_nullFields_handledGracefully() {
        String json = "{"
                + "\"nextHeartbeatIntervalMs\":1000,"
                + "\"operation\":\"Continue\""
                + "}";

        Gson gson = new Gson();
        SessionHostHeartbeatInfo info = gson.fromJson(json, SessionHostHeartbeatInfo.class);

        assertNull(info.getSessionConfig());
        assertNull(info.getNextScheduledMaintenanceUtc());
        assertNull(info.getMaintenanceSchedule());
        assertNull(info.getCurrentPlayers());
    }

    @Test
    public void serialization_heartbeatRequest_producesExpectedJson() {
        SessionHostHeartbeatInfo info = new SessionHostHeartbeatInfo();
        info.setCurrentGameState(SessionHostStatus.StandingBy);
        info.setCurrentGameHealth(GameHostHealth.Healthy);

        Gson gson = new Gson();
        String json = gson.toJson(info);

        assertTrue(json.contains("\"currentGameState\":\"StandingBy\""));
        assertTrue(json.contains("\"currentGameHealth\":\"Healthy\""));
    }
}

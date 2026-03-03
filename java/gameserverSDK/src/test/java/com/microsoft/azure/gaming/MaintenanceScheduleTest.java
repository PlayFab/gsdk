package com.microsoft.azure.gaming;

import com.google.gson.*;
import org.junit.Test;

import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.Arrays;
import java.util.List;

import static org.junit.Assert.*;

/**
 * Tests for MaintenanceSchedule and MaintenanceEvent deserialization.
 */
public class MaintenanceScheduleTest {

    private Gson createGsonWithDateSupport() {
        return new GsonBuilder().registerTypeAdapter(ZonedDateTime.class,
                (JsonDeserializer<ZonedDateTime>) (json, type, ctx) ->
                        ZonedDateTime.parse(json.getAsJsonPrimitive().getAsString())
                                .withZoneSameLocal(ZoneId.of("UTC"))).create();
    }

    @Test
    public void deserialization_maintenanceSchedule_parsesDocumentIncarnation() {
        String json = "{"
                + "\"documentIncarnation\":\"IncarnationID\","
                + "\"Events\":[]"
                + "}";

        MaintenanceSchedule schedule = new Gson().fromJson(json, MaintenanceSchedule.class);
        assertEquals("IncarnationID", schedule.getDocumentIncarnation());
    }

    @Test
    public void deserialization_maintenanceSchedule_parsesEvents() {
        String json = "{"
                + "\"documentIncarnation\":\"IncarnationID\","
                + "\"Events\":[{"
                + "  \"eventId\":\"eventID\","
                + "  \"eventType\":\"Reboot\","
                + "  \"resourceType\":\"VirtualMachine\","
                + "  \"Resources\":[\"resourceName\"],"
                + "  \"eventStatus\":\"Scheduled\","
                + "  \"notBefore\":\"2024-01-15T10:30:00Z\","
                + "  \"description\":\"eventDescription\","
                + "  \"eventSource\":\"Platform\","
                + "  \"durationInSeconds\":3600"
                + "}]"
                + "}";

        Gson gson = createGsonWithDateSupport();
        MaintenanceSchedule schedule = gson.fromJson(json, MaintenanceSchedule.class);

        assertNotNull(schedule.getMaintenanceEvents());
        assertEquals(1, schedule.getMaintenanceEvents().size());

        MaintenanceEvent event = schedule.getMaintenanceEvents().get(0);
        assertEquals("eventID", event.getEventId());
        assertEquals("Reboot", event.getEventType());
        assertEquals("VirtualMachine", event.getResourceType());
        assertEquals(1, event.getResources().size());
        assertEquals("resourceName", event.getResources().get(0));
        assertEquals("Scheduled", event.getEventStatus());
        assertEquals("eventDescription", event.getDescription());
        assertEquals("Platform", event.getEventSource());
        assertEquals(3600, event.getDurationInSeconds());
    }

    @Test
    public void deserialization_maintenanceEvent_parsesNotBeforeDate() {
        String json = "{"
                + "\"eventId\":\"eventID\","
                + "\"notBefore\":\"2024-01-15T10:30:00Z\""
                + "}";

        Gson gson = createGsonWithDateSupport();
        MaintenanceEvent event = gson.fromJson(json, MaintenanceEvent.class);

        assertNotNull(event.getNotBefore());
        assertEquals(2024, event.getNotBefore().getYear());
        assertEquals(1, event.getNotBefore().getMonthValue());
        assertEquals(15, event.getNotBefore().getDayOfMonth());
        assertEquals(10, event.getNotBefore().getHour());
        assertEquals(30, event.getNotBefore().getMinute());
    }

    @Test
    public void deserialization_multipleEvents_parsesAll() {
        String json = "{"
                + "\"documentIncarnation\":\"Inc1\","
                + "\"Events\":["
                + "  {\"eventId\":\"event1\",\"eventType\":\"Reboot\",\"durationInSeconds\":1800},"
                + "  {\"eventId\":\"event2\",\"eventType\":\"Freeze\",\"durationInSeconds\":600}"
                + "]"
                + "}";

        MaintenanceSchedule schedule = new Gson().fromJson(json, MaintenanceSchedule.class);

        assertEquals(2, schedule.getMaintenanceEvents().size());
        assertEquals("event1", schedule.getMaintenanceEvents().get(0).getEventId());
        assertEquals("Reboot", schedule.getMaintenanceEvents().get(0).getEventType());
        assertEquals(1800, schedule.getMaintenanceEvents().get(0).getDurationInSeconds());
        assertEquals("event2", schedule.getMaintenanceEvents().get(1).getEventId());
        assertEquals("Freeze", schedule.getMaintenanceEvents().get(1).getEventType());
        assertEquals(600, schedule.getMaintenanceEvents().get(1).getDurationInSeconds());
    }

    @Test
    public void deserialization_emptyEvents_parsesEmptyList() {
        String json = "{"
                + "\"documentIncarnation\":\"Inc1\","
                + "\"Events\":[]"
                + "}";

        MaintenanceSchedule schedule = new Gson().fromJson(json, MaintenanceSchedule.class);

        assertNotNull(schedule.getMaintenanceEvents());
        assertEquals(0, schedule.getMaintenanceEvents().size());
    }

    @Test
    public void deserialization_multipleResources_parsesAll() {
        String json = "{"
                + "\"eventId\":\"event1\","
                + "\"Resources\":[\"resource1\",\"resource2\",\"resource3\"]"
                + "}";

        MaintenanceEvent event = new Gson().fromJson(json, MaintenanceEvent.class);

        assertEquals(3, event.getResources().size());
        assertEquals("resource1", event.getResources().get(0));
        assertEquals("resource2", event.getResources().get(1));
        assertEquals("resource3", event.getResources().get(2));
    }

    @Test
    public void deserialization_inHeartbeatResponse_parsesMaintenanceSchedule() {
        String json = "{"
                + "\"nextHeartbeatIntervalMs\":1000,"
                + "\"operation\":\"Continue\","
                + "\"maintenanceSchedule\":{"
                + "  \"documentIncarnation\":\"Inc1\","
                + "  \"Events\":[{"
                + "    \"eventId\":\"event1\","
                + "    \"eventType\":\"Reboot\","
                + "    \"durationInSeconds\":3600"
                + "  }]"
                + "}"
                + "}";

        SessionHostHeartbeatInfo info = new Gson().fromJson(json, SessionHostHeartbeatInfo.class);

        assertNotNull(info.getMaintenanceSchedule());
        assertEquals("Inc1", info.getMaintenanceSchedule().getDocumentIncarnation());
        assertEquals(1, info.getMaintenanceSchedule().getMaintenanceEvents().size());
        assertEquals("event1", info.getMaintenanceSchedule().getMaintenanceEvents().get(0).getEventId());
    }
}

package com.microsoft.azure.gaming;

import java.time.ZonedDateTime;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import com.google.gson.annotations.SerializedName;

public class MaintenanceEvent {
    
    private String eventId;

    private String eventType;

    private String resourceType;

    @SerializedName(value = "Resources")
    private List<String> resources;

    private String eventStatus;

    private ZonedDateTime notBefore;

    private String description;

    private String eventSource;

    private int durationInSeconds;

    public String getEventId() {
        return eventId;
    }

    public String getEventType() {
        return eventType;
    }

    public String getResourceType() {
        return resourceType;
    }

    public List<String> getResources() {
        return resources;
    }

    public String getEventStatus() {
        return eventStatus;
    }

    public ZonedDateTime getNotBefore() {
        return notBefore;
    }

    public String getDescription() {
        return description;
    }

    public String getEventSource() {
        return eventSource;
    }

    public int getDurationInSeconds() {
        return durationInSeconds;
    }
}
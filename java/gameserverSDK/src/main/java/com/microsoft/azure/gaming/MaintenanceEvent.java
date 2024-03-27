package com.microsoft.azure.gaming;

import java.time.ZonedDateTime;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import com.google.gson.annotations.SerializedName;

public class MaintenanceEvent {
    
    @SerializedName("EventId")
    private String eventId;

    @SerializedName("EventType")
    private String eventType;

    @SerializedName("ResourceType")
    private String resourceType;

    @SerializedName("Resources")
    private List<String> resources;

    @SerializedName("EventStatus")
    private String eventStatus;

    @SerializedName("NotBefore")
    private ZonedDateTime notBefore;

    @SerializedName("Description")
    private String description;

    @SerializedName("EventSource")
    private String eventSource;

    @SerializedName("DurationInSeconds")
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
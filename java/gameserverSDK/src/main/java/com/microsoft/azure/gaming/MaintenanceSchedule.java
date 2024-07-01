package com.microsoft.azure.gaming;

import java.util.List;
import com.google.gson.annotations.SerializedName;

public class MaintenanceSchedule {
    
    private String documentIncarnation;

    @SerializedName(value = "Events")
    private List<MaintenanceEvent> events;

    public String getDocumentIncarnation() {
        return documentIncarnation;
    }

    public List<MaintenanceEvent> getMaintenanceEvents() {
        return events;
    }
}
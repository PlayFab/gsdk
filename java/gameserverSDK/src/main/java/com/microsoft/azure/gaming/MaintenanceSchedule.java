package com.microsoft.azure.gaming;

import java.util.List;
import com.google.gson.annotations.SerializedName;

public class MaintenanceSchedule {
    
    @SerializedName("DocumentIncarnation")
    private String documentIncarnation;

    @SerializedName("Events")
    private List<MaintenanceEvent> events;

    public String getDocumentIncarnation() {
        return documentIncarnation;
    }

    public List<MaintenanceEvent> getMaintenanceEvents() {
        return events;
    }
}
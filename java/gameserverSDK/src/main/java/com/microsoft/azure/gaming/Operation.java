package com.microsoft.azure.gaming;

import com.google.gson.annotations.SerializedName;

/**
 * Created by hamonciv on 7/17/2017.
 * Note: The order needs to match the VM Agent and C++/C# GSDK.
 * Also make sure to keep these Uppercase, since Java string to
 * enum parsing is case-sensitive
 */
public enum Operation {
    @SerializedName(value = "invalid", alternate = {"Invalid", "INVALID"})
    INVALID,

    @SerializedName(value = "continue", alternate = {"Continue", "CONTINUE"})
    CONTINUE,

    @SerializedName(value = "getmanifest", alternate = {"GetManifest", "GETMANIFEST"})
    GETMANIFEST,

    // TODO: Remove when CP no longer has Quarantine
    @SerializedName(value = "quarantine", alternate = {"Quarantine", "QUARANTINE"})
    QUARANTINE,

    @SerializedName(value = "active", alternate = {"Active", "ACTIVE"})
    ACTIVE,

    @SerializedName(value = "terminate", alternate = {"Terminate", "TERMINATE"})
    TERMINATE
}

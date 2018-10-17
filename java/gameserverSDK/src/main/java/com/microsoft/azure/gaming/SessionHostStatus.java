package com.microsoft.azure.gaming;

/**
 * The series of game states we support.
 * Note: This needs to match the VM Agent and C++/C# GSDK
 *
 * Created by hamonciv on 7/17/2017.
 */
public enum SessionHostStatus {
    Invalid,
    Initializing,
    StandingBy,
    Active,
    Terminating,
    Terminated,
    Quarantined, // TODO: Remove when CP no longer has Quarantine
}

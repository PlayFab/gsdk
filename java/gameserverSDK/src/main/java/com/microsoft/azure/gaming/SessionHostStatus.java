package com.microsoft.azure.gaming;

/**
 * The series of game states we support.
 * Note: This needs to match the VM Agent and C++/C# GSDK
 */
enum SessionHostStatus {
    Invalid,
    Initializing,
    StandingBy,
    Active,
    Terminating,
    Terminated,
    Quarantined, // TODO: Remove when CP no longer has Quarantine
}

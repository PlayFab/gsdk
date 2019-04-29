package com.microsoft.azure.gaming;

import java.util.Map;

/**
 * Various properties that can be configured for the GSDK.
 * As long as it implements this interface, the GSDK can accept configurations
 * from various sources, such as environment variables, json files, etc.
 */
interface Configuration {
    String getHeartbeatEndpoint();
    String getServerId();
    String getLogFolder();
    String getSharedContentFolder();
    String getCertificateFolder();
    String getVmId();
    String getTitleId();
    String getBuildId();
    String getRegion();
    Map<String, String> getBuildMetadata();
    Map<String, String> getGamePorts();
    void validate() throws GameserverSDKInitializationException;


    final String TITLE_ID_VARIABLE_NAME = "PF_TITLE_ID";
    final String BUILD_ID_VARIABLE_NAME = "PF_BUILD_ID";
    final String REGION_VARIABLE_NAME = "PF_REGION";
}

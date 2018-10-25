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
    String getCertificateFolder();
    String getTitleId();
    String getBuildId();
    String getRegion();
    Map<String, String> getBuildMetadata();
    void validate() throws GameserverSDKInitializationException;


    final String HEARTBEAT_ENDPOINT_VARIABLE_NAME = "HEARTBEAT_ENDPOINT";
    final String SERVER_ID_VARIABLE_NAME = "SESSION_HOST_ID";
    final String LOG_FOLDER_VARIABLE_NAME = "GSDK_LOG_FOLDER";
    final String TITLE_ID_VARIABLE_NAME = "PF_TITLE_ID";
    final String BUILD_ID_VARIABLE_NAME = "PF_BUILD_ID";
    final String REGION_VARIABLE_NAME = "PF_REGION";
}

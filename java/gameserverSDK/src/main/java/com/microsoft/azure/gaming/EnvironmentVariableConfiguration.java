package com.microsoft.azure.gaming;

import java.util.HashMap;
import java.util.Map;

/**
 * Grabs all configuration values from environment variables
 */
public class EnvironmentVariableConfiguration extends ConfigurationBase {
    private final String heartbeatEndpoint;
    private final String serverId;
    private final String logFolder;
    private final String certFolder;
    private final Map<String, String> buildMetadata;

    public EnvironmentVariableConfiguration(){
        this.heartbeatEndpoint = System.getenv(HEARTBEAT_ENDPOINT_VARIABLE_NAME);
        this.serverId = System.getenv(SERVER_ID_VARIABLE_NAME);
        this.logFolder = System.getenv(LOG_FOLDER_VARIABLE_NAME);

        // Certificate and metadata support was added once we moved to a json config
        certFolder = "";
        buildMetadata = new HashMap<String, String>();
    }
    @Override
    public String getHeartbeatEndpoint() {
        return this.heartbeatEndpoint;
    }

    @Override
    public String getServerId() {
        return this.serverId;
    }

    @Override
    public String getLogFolder() {
        return this.logFolder;
    }

    @Override
    public String getCertificateFolder() { return this.certFolder; }

    @Override
    public Map<String, String> getBuildMetadata() { return this.buildMetadata; }

    @Override
    public void validate() throws GameserverSDKInitializationException {
        if((this.heartbeatEndpoint == null || this.heartbeatEndpoint.isEmpty()) &&
                (this.serverId == null || this.serverId.isEmpty())){
            String errorMessage = "Must provide a value for both the '" + HEARTBEAT_ENDPOINT_VARIABLE_NAME + "' and the '" + SERVER_ID_VARIABLE_NAME + "' environment variables.";
            Logger.Instance().LogError(errorMessage);
            throw new GameserverSDKInitializationException(errorMessage);
        }
        else if(this.heartbeatEndpoint == null || this.heartbeatEndpoint.isEmpty()){
            String errorMessage = "Must provide a value for the '" + HEARTBEAT_ENDPOINT_VARIABLE_NAME + "' environment variable.";
            Logger.Instance().LogError(errorMessage);
            throw new GameserverSDKInitializationException(errorMessage);
        }
        else if(this.serverId == null || this.serverId.isEmpty()){
            String errorMessage = "Must provide a value for the '" + SERVER_ID_VARIABLE_NAME + "' environment variable.";
            Logger.Instance().LogError(errorMessage);
            throw new GameserverSDKInitializationException(errorMessage);
        }
    }
}

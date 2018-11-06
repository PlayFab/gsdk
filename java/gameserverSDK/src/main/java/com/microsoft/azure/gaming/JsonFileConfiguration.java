package com.microsoft.azure.gaming;

import com.google.gson.Gson;
import com.google.gson.annotations.SerializedName;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Map;

/**
 * Grabs configuration values from a json config file
 */
class JsonFileConfiguration extends ConfigurationBase {
    protected static final String CONFIG_FILE_VARIABLE_NAME = "GSDK_CONFIG_FILE";

    private JsonConfigurationFields configValues;

    protected JsonFileConfiguration(){
        Path configFilePath = Paths.get(System.getenv(CONFIG_FILE_VARIABLE_NAME));
        try {
            // Gson doesn't have a way of populating "this" object with json, so we use a helper inner class
            // to specify the json file format
            Gson gson = new Gson();
            String json = new String(Files.readAllBytes(configFilePath), "UTF-8");
            this.configValues = gson.fromJson(json, JsonConfigurationFields.class);
        } catch (IOException e) {
            StringWriter errors = new StringWriter();
            e.printStackTrace(new PrintWriter(errors));
            String errorMessage = "Could not read configuration file: " + configFilePath.toString() + " Exception: " + errors.toString();
            Logger.Instance().LogError(errorMessage);
            throw new GameserverSDKInitializationException(errorMessage);
        }
    }

    @Override
    public String getHeartbeatEndpoint() { return configValues.getHeartbeatEndpoint(); }

    @Override
    public String getServerId() { return configValues.getServerId(); }

    @Override
    public String getLogFolder() { return configValues.getLogFolder(); }

    @Override
    public String getCertificateFolder() { return configValues.getCertificateFolder(); }

    @Override
    public Map<String, String> getBuildMetadata() { return configValues.getBuildMetadata(); }

    @Override
    public Map<String, String> getGamePorts() { return configValues.getGamePorts(); }

    @Override
    public void validate() throws GameserverSDKInitializationException {
        if((this.getHeartbeatEndpoint() == null || this.getHeartbeatEndpoint().isEmpty()) &&
                (this.getServerId() == null || this.getServerId().isEmpty())){
            String errorMessage = "Config file must include values for the heartbeatEndpoint and the sessionHostId.";
            Logger.Instance().LogError(errorMessage);
            throw new GameserverSDKInitializationException(errorMessage);
        }
        else if(this.getHeartbeatEndpoint() == null || this.getHeartbeatEndpoint().isEmpty()){
            String errorMessage = "Config file must include a value for the heartbeatEndpoint.";
            Logger.Instance().LogError(errorMessage);
            throw new GameserverSDKInitializationException(errorMessage);
        }
        else if(this.getServerId() == null || this.getServerId().isEmpty()){
            String errorMessage = "Config file must include a value for the sessionHostId.";
            Logger.Instance().LogError(errorMessage);
            throw new GameserverSDKInitializationException(errorMessage);
        }
    }

    /**
     * Helper class to hold the fields that we expect to see
     * in our json config file
     */
    class JsonConfigurationFields
    {
        @SerializedName(value = "heartbeatEndpoint", alternate = {"HeartbeatEndpoint", "HEARTBEATENDPOINT", "heartbeatendpoint"})
        String heartbeatEndpoint;

        @SerializedName(value = "sessionHostId", alternate = {"SessionHostId", "SESSIONHOSTID", "sessionhostid"})
        String sessionHostId;

        @SerializedName(value = "logFolder", alternate = {"LogFolder", "LOGFOLDER", "logfolder"})
        String logFolder;

        @SerializedName(value = "certificateFolder", alternate = {"CertificateFolder", "CERTIFICATEFOLDER", "certificatefolder"})
        String certificateFolder;

        @SerializedName(value = "buildMetadata", alternate = {"BuildMetadata", "BUILDMETADATA", "buildmetadata"})
        Map<String, String> buildMetadata;

            @SerializedName(value = "gamePorts", alternate = {"GamePorts", "GAMEPORTS", "gameports"})
        Map<String, String> gamePorts;

        public String getHeartbeatEndpoint() {
            return heartbeatEndpoint;
        }

        public String getServerId() {
            return sessionHostId;
        }

        public String getLogFolder() {
            return logFolder;
        }

        public String getCertificateFolder() { return certificateFolder; }

        public Map<String, String> getBuildMetadata() { return buildMetadata; }

        public Map<String, String> getGamePorts() { return gamePorts; }
    }
}

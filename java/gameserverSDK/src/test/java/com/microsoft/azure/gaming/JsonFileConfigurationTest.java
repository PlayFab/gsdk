package com.microsoft.azure.gaming;

import com.google.gson.Gson;
import com.google.gson.annotations.SerializedName;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import java.io.File;
import java.io.FileWriter;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * Tests for JsonFileConfiguration config file parsing and validation.
 * Uses a temp config file and environment variable manipulation to test.
 */
public class JsonFileConfigurationTest {

    @Rule
    public TemporaryFolder tempFolder = new TemporaryFolder();

    private void setEnvironmentVariable(String key, String value) throws Exception {
        Map<String, String> env = System.getenv();
        Field field = env.getClass().getDeclaredField("m");
        field.setAccessible(true);
        @SuppressWarnings("unchecked")
        Map<String, String> writableEnv = (Map<String, String>) field.get(env);
        if (value == null) {
            writableEnv.remove(key);
        } else {
            writableEnv.put(key, value);
        }
    }

    private File createConfigFile(String jsonContent) throws Exception {
        File configFile = tempFolder.newFile("gsdkConfig.json");
        try (FileWriter writer = new FileWriter(configFile)) {
            writer.write(jsonContent);
        }
        return configFile;
    }

    @Before
    public void setUp() throws Exception {
        // Clean up any previous env vars
        setEnvironmentVariable("PF_TITLE_ID", "testTitleId");
        setEnvironmentVariable("PF_BUILD_ID", "testBuildId");
        setEnvironmentVariable("PF_REGION", "WestUS");
    }

    @Test
    public void validConfig_parsesAllFields() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"server123\","
                + "\"logFolder\":\"/logs\","
                + "\"sharedContentFolder\":\"/shared\","
                + "\"certificateFolder\":\"/certs\","
                + "\"vmId\":\"vm-001\","
                + "\"buildMetadata\":{\"key1\":\"value1\",\"key2\":\"value2\"},"
                + "\"gamePorts\":{\"gamePort\":\"8080\",\"queryPort\":\"27015\"}"
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();

        assertEquals("localhost:56001", config.getHeartbeatEndpoint());
        assertEquals("server123", config.getServerId());
        assertEquals("/logs", config.getLogFolder());
        assertEquals("/shared", config.getSharedContentFolder());
        assertEquals("/certs", config.getCertificateFolder());
        assertEquals("vm-001", config.getVmId());
        assertNotNull(config.getBuildMetadata());
        assertEquals("value1", config.getBuildMetadata().get("key1"));
        assertEquals("value2", config.getBuildMetadata().get("key2"));
        assertNotNull(config.getGamePorts());
        assertEquals("8080", config.getGamePorts().get("gamePort"));
        assertEquals("27015", config.getGamePorts().get("queryPort"));
    }

    @Test
    public void validConfig_alternateKeyCasing_HeartbeatEndpoint() throws Exception {
        String json = "{"
                + "\"HeartbeatEndpoint\":\"localhost:56001\","
                + "\"SessionHostId\":\"server123\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();

        assertEquals("localhost:56001", config.getHeartbeatEndpoint());
        assertEquals("server123", config.getServerId());
    }

    @Test
    public void validConfig_allUpperCaseKeys() throws Exception {
        String json = "{"
                + "\"HEARTBEATENDPOINT\":\"localhost:56001\","
                + "\"SESSIONHOSTID\":\"server123\","
                + "\"LOGFOLDER\":\"/logs\","
                + "\"CERTIFICATEFOLDER\":\"/certs\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();

        assertEquals("localhost:56001", config.getHeartbeatEndpoint());
        assertEquals("server123", config.getServerId());
        assertEquals("/logs", config.getLogFolder());
        assertEquals("/certs", config.getCertificateFolder());
    }

    @Test
    public void validConfig_readsTitleIdFromEnv() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"server123\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());
        setEnvironmentVariable("PF_TITLE_ID", "myTitleId");
        setEnvironmentVariable("PF_BUILD_ID", "myBuildId");
        setEnvironmentVariable("PF_REGION", "EastUS");

        JsonFileConfiguration config = new JsonFileConfiguration();

        assertEquals("myTitleId", config.getTitleId());
        assertEquals("myBuildId", config.getBuildId());
        assertEquals("EastUS", config.getRegion());
    }

    @Test
    public void validate_validConfig_doesNotThrow() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"server123\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        config.validate(); // Should not throw
    }

    @Test(expected = GameserverSDKInitializationException.class)
    public void validate_missingBothEndpointAndServerId_throws() throws Exception {
        String json = "{}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        config.validate();
    }

    @Test(expected = GameserverSDKInitializationException.class)
    public void validate_missingHeartbeatEndpoint_throws() throws Exception {
        String json = "{\"sessionHostId\":\"server123\"}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        config.validate();
    }

    @Test(expected = GameserverSDKInitializationException.class)
    public void validate_missingServerId_throws() throws Exception {
        String json = "{\"heartbeatEndpoint\":\"localhost:56001\"}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        config.validate();
    }

    @Test(expected = GameserverSDKInitializationException.class)
    public void validate_emptyHeartbeatEndpoint_throws() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"\","
                + "\"sessionHostId\":\"server123\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        config.validate();
    }

    @Test(expected = GameserverSDKInitializationException.class)
    public void validate_emptyServerId_throws() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        config.validate();
    }

    @Test(expected = GameserverSDKInitializationException.class)
    public void validate_emptyBothFields_throws() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"\","
                + "\"sessionHostId\":\"\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        config.validate();
    }

    @Test
    public void validConfig_nullBuildMetadata_returnsNull() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"server123\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        assertNull(config.getBuildMetadata());
    }

    @Test
    public void validConfig_nullGamePorts_returnsNull() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"server123\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();
        assertNull(config.getGamePorts());
    }

    @Test(expected = com.google.gson.JsonSyntaxException.class)
    public void invalidJsonFile_throwsJsonSyntaxException() throws Exception {
        String invalidJson = "not valid json {{{";

        File configFile = createConfigFile(invalidJson);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        new JsonFileConfiguration();
    }

    @Test
    public void validConfig_alternateKeyCasing_BuildMetadata() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"server123\","
                + "\"BuildMetadata\":{\"property1\":\"value1\"}"
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();

        assertNotNull(config.getBuildMetadata());
        assertEquals("value1", config.getBuildMetadata().get("property1"));
    }

    @Test
    public void validConfig_alternateKeyCasing_VmId() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"server123\","
                + "\"VmId\":\"vm-test-001\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();

        assertEquals("vm-test-001", config.getVmId());
    }

    @Test
    public void validConfig_alternateKeyCasing_SharedContentFolder() throws Exception {
        String json = "{"
                + "\"heartbeatEndpoint\":\"localhost:56001\","
                + "\"sessionHostId\":\"server123\","
                + "\"SharedContentFolder\":\"/shared/content\""
                + "}";

        File configFile = createConfigFile(json);
        setEnvironmentVariable(JsonFileConfiguration.CONFIG_FILE_VARIABLE_NAME, configFile.getAbsolutePath());

        JsonFileConfiguration config = new JsonFileConfiguration();

        assertEquals("/shared/content", config.getSharedContentFolder());
    }
}

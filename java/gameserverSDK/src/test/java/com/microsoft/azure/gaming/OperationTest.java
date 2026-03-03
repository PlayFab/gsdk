package com.microsoft.azure.gaming;

import com.google.gson.Gson;
import org.junit.Test;

import static org.junit.Assert.*;

/**
 * Tests for the Operation enum, especially Gson deserialization
 * with the @SerializedName annotations that support multiple casings.
 */
public class OperationTest {

    @Test
    public void deserialization_lowercaseContinue() {
        String json = "\"continue\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.CONTINUE, op);
    }

    @Test
    public void deserialization_titleCaseContinue() {
        String json = "\"Continue\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.CONTINUE, op);
    }

    @Test
    public void deserialization_uppercaseContinue() {
        String json = "\"CONTINUE\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.CONTINUE, op);
    }

    @Test
    public void deserialization_lowercaseActive() {
        String json = "\"active\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.ACTIVE, op);
    }

    @Test
    public void deserialization_titleCaseActive() {
        String json = "\"Active\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.ACTIVE, op);
    }

    @Test
    public void deserialization_uppercaseActive() {
        String json = "\"ACTIVE\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.ACTIVE, op);
    }

    @Test
    public void deserialization_lowercaseTerminate() {
        String json = "\"terminate\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.TERMINATE, op);
    }

    @Test
    public void deserialization_titleCaseTerminate() {
        String json = "\"Terminate\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.TERMINATE, op);
    }

    @Test
    public void deserialization_uppercaseTerminate() {
        String json = "\"TERMINATE\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.TERMINATE, op);
    }

    @Test
    public void deserialization_lowercaseInvalid() {
        String json = "\"invalid\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.INVALID, op);
    }

    @Test
    public void deserialization_titleCaseInvalid() {
        String json = "\"Invalid\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.INVALID, op);
    }

    @Test
    public void deserialization_lowercaseGetManifest() {
        String json = "\"getmanifest\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.GETMANIFEST, op);
    }

    @Test
    public void deserialization_titleCaseGetManifest() {
        String json = "\"GetManifest\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.GETMANIFEST, op);
    }

    @Test
    public void deserialization_lowercaseQuarantine() {
        String json = "\"quarantine\"";
        Operation op = new Gson().fromJson(json, Operation.class);
        assertEquals(Operation.QUARANTINE, op);
    }

    @Test
    public void serialization_producesLowercaseValue() {
        Gson gson = new Gson();
        String json = gson.toJson(Operation.CONTINUE);
        assertEquals("\"continue\"", json);
    }

    @Test
    public void serialization_activeProducesLowercaseValue() {
        Gson gson = new Gson();
        String json = gson.toJson(Operation.ACTIVE);
        assertEquals("\"active\"", json);
    }

    @Test
    public void serialization_terminateProducesLowercaseValue() {
        Gson gson = new Gson();
        String json = gson.toJson(Operation.TERMINATE);
        assertEquals("\"terminate\"", json);
    }

    @Test
    public void allEnumValues_exist() {
        Operation[] values = Operation.values();
        assertEquals(6, values.length);
    }

    @Test
    public void deserialization_inHeartbeatResponse_parsesCorrectly() {
        String json = "{\"operation\":\"Active\",\"nextHeartbeatIntervalMs\":1000}";
        Gson gson = new Gson();
        SessionHostHeartbeatInfo info = gson.fromJson(json, SessionHostHeartbeatInfo.class);
        assertEquals(Operation.ACTIVE, info.getOperation());
    }

    @Test
    public void deserialization_inHeartbeatResponse_lowercaseOperation() {
        String json = "{\"operation\":\"terminate\",\"nextHeartbeatIntervalMs\":1000}";
        Gson gson = new Gson();
        SessionHostHeartbeatInfo info = gson.fromJson(json, SessionHostHeartbeatInfo.class);
        assertEquals(Operation.TERMINATE, info.getOperation());
    }
}

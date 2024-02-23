// Copyright (C) Microsoft Corporation. All rights reserved.

#if (WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR)
#include "TestConfiguration.h"
#include "Logging/LogMacros.h"
#include "PlayFabGSDK.h"

void TestConfiguration::SetHeartbeatEndpoint(FString heartbeatEndpoint)
{
    HeartbeatEndpoint = heartbeatEndpoint;
}

void TestConfiguration::SetServerId(FString serverId)
{
    ServerId = serverId;
}

void TestConfiguration::SetLogFolder(FString logFolder)
{
    LogFolder = logFolder;
}

void TestConfiguration::SetCertificateFolder(FString certFolder)
{
    CertFolder = certFolder;
}

void TestConfiguration::SetSharedContentFolder(FString sharedContentFolder)
{
    SharedContentFolder = sharedContentFolder;
}

void TestConfiguration::SetGameCertificates(TMap<FString, FString> gameCerts)
{
    GameCerts = gameCerts;
}

void TestConfiguration::SetBuildMetadata(TMap<FString, FString> metadata)
{
    Metadata = metadata;
}

void TestConfiguration::SetGamePorts(TMap<FString, FString> ports)
{
    Ports = ports;
}

void TestConfiguration::SetPublicIpV4Address(FString ipv4Address)
{
    Ipv4Address = ipv4Address;
}

void TestConfiguration::SetFullyQualifiedDomainName(FString domainName)
{
    DomainName = domainName;
}

void TestConfiguration::SetGameServerConnectionInfo(FGameServerConnectionInfo connectionInfo)
{
    ConnectionInfo = connectionInfo;
}

bool TestConfiguration::SerializeToFile(const FString& filePath)
{
    // Serialize all configuration properties to a JSON file
    TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
    jsonObject->SetStringField("heartbeatEndpoint", HeartbeatEndpoint);
    jsonObject->SetStringField("sessionHostId", ServerId);
    jsonObject->SetStringField("logFolder", LogFolder);
    jsonObject->SetStringField("certificateFolder", CertFolder);
    jsonObject->SetStringField("sharedContentFolder", SharedContentFolder);
    jsonObject->SetStringField("publicIpV4Address", Ipv4Address);
    jsonObject->SetStringField("fullyQualifiedDomainName", DomainName);

    // Serialize game certificates
    TSharedPtr<FJsonObject> gameCertsObject = MakeShareable(new FJsonObject);
    for (const auto& cert : GameCerts)
    {
        gameCertsObject->SetStringField(cert.Key, cert.Value);
    }
    jsonObject->SetObjectField("gameCertificates", gameCertsObject);

    // Serialize build metadata
    TSharedPtr<FJsonObject> metadataObject = MakeShareable(new FJsonObject);
    for (const auto& meta : Metadata)
    {
        metadataObject->SetStringField(meta.Key, meta.Value);
    }
    jsonObject->SetObjectField("buildMetadata", metadataObject);

    // Serialize game ports
    TSharedPtr<FJsonObject> portsObject = MakeShareable(new FJsonObject);
    for (const auto& port : Ports)
    {
        portsObject->SetStringField(port.Key, port.Value);
    }
    jsonObject->SetObjectField("gamePorts", portsObject);

    // Serialize game server connection info
    TSharedPtr<FJsonObject> connectionInfoObject = MakeShareable(new FJsonObject);
    connectionInfoObject->SetStringField("publicIpV4Adress", ConnectionInfo.PublicIpV4Address);

    // Serialize game ports configuration
    TArray<TSharedPtr<FJsonValue>> gamePortsConfiguration;
    for (const auto& portConfig : ConnectionInfo.GamePortsConfiguration)
    {
        TSharedPtr<FJsonObject> portObject = MakeShareable(new FJsonObject);
        portObject->SetStringField("name", portConfig.Name);
        portObject->SetNumberField("serverListeningPort", portConfig.ServerListeningPort);
        portObject->SetNumberField("clientConnectionPort", portConfig.ClientConnectionPort);

        TSharedRef<FJsonValueObject> portValueObject = MakeShareable(new FJsonValueObject(portObject));
        gamePortsConfiguration.Add(portValueObject);
    }

    connectionInfoObject->SetArrayField("gamePortsConfiguration", gamePortsConfiguration);
    jsonObject->SetObjectField("gameServerConnectionInfo", connectionInfoObject);

    // Serialize the JSON object to a string
    FString jsonString;
    TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&jsonString);
    FJsonSerializer::Serialize(jsonObject.ToSharedRef(), writer);
    
    // Write the JSON string to a file
    return FFileHelper::SaveStringToFile(jsonString, *filePath);
}
#endif

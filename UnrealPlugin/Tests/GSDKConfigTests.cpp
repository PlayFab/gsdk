// Copyright (C) Microsoft Corporation. All rights reserved.

// Standalone unit tests for the Unreal GSDK configuration JSON format.
// These tests validate the JSON contracts used by FJsonFileConfiguration
// and TestConfiguration without requiring Unreal Engine.

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>
#include <vector>

using json = nlohmann::json;

namespace {

// Mirrors TestConfiguration::SerializeToFile
// Creates a JSON config in the format expected by FJsonFileConfiguration
json SerializeConfig(
	const std::string& heartbeatEndpoint,
	const std::string& serverId,
	const std::string& logFolder,
	const std::string& sharedContentFolder,
	const std::string& certFolder = "",
	const std::map<std::string, std::string>& gameCerts = {},
	const std::map<std::string, std::string>& metadata = {},
	const std::map<std::string, std::string>& ports = {},
	const std::string& publicIpV4Address = "",
	const std::string& fullyQualifiedDomainName = "",
	const std::string& vmId = "")
{
	json config;
	config["heartbeatEndpoint"] = heartbeatEndpoint;
	config["sessionHostId"] = serverId;
	config["logFolder"] = logFolder;
	config["sharedContentFolder"] = sharedContentFolder;
	config["certificateFolder"] = certFolder;
	config["publicIpV4Address"] = publicIpV4Address;
	config["fullyQualifiedDomainName"] = fullyQualifiedDomainName;
	config["vmId"] = vmId;

	config["gameCertificates"] = json::object();
	for (const auto& [key, value] : gameCerts)
	{
		config["gameCertificates"][key] = value;
	}

	config["buildMetadata"] = json::object();
	for (const auto& [key, value] : metadata)
	{
		config["buildMetadata"][key] = value;
	}

	config["gamePorts"] = json::object();
	for (const auto& [key, value] : ports)
	{
		config["gamePorts"][key] = value;
	}

	config["gameServerConnectionInfo"] = {
		{"publicIpV4Adress", publicIpV4Address}, // Note: matches typo in UE code
		{"gamePortsConfiguration", json::array()}
	};

	return config;
}

// Mirrors FJsonFileConfiguration constructor
// Parses configuration from JSON in the same way the Unreal GSDK does
struct ParsedConfig
{
	std::string heartbeatEndpoint;
	std::string serverId;
	std::string logFolder;
	std::string sharedContentFolder;
	std::string certFolder;
	std::map<std::string, std::string> gameCerts;
	std::map<std::string, std::string> metadata;
	std::map<std::string, std::string> ports;
	std::string publicIpV4Address;
	std::string fullyQualifiedDomainName;
	std::string vmId;

	struct GamePort
	{
		std::string name;
		int serverListeningPort = 0;
		int clientConnectionPort = 0;
	};

	struct ConnectionInfo
	{
		std::string publicIpV4Address;
		std::vector<GamePort> gamePortsConfiguration;
	};

	ConnectionInfo connectionInfo;
};

ParsedConfig ParseConfig(const json& config)
{
	ParsedConfig parsed;

	parsed.heartbeatEndpoint = config.value("heartbeatEndpoint", "");
	parsed.serverId = config.value("sessionHostId", "");
	parsed.logFolder = config.value("logFolder", "");
	parsed.sharedContentFolder = config.value("sharedContentFolder", "");

	if (config.contains("certificateFolder"))
	{
		parsed.certFolder = config["certificateFolder"].get<std::string>();
	}

	if (config.contains("gameCertificates"))
	{
		for (const auto& [key, value] : config["gameCertificates"].items())
		{
			parsed.gameCerts[key] = value.get<std::string>();
		}
	}

	if (config.contains("buildMetadata"))
	{
		for (const auto& [key, value] : config["buildMetadata"].items())
		{
			parsed.metadata[key] = value.get<std::string>();
		}
	}

	if (config.contains("gamePorts"))
	{
		for (const auto& [key, value] : config["gamePorts"].items())
		{
			parsed.ports[key] = value.get<std::string>();
		}
	}

	if (config.contains("publicIpV4Address"))
	{
		parsed.publicIpV4Address = config["publicIpV4Address"].get<std::string>();
	}

	if (config.contains("fullyQualifiedDomainName"))
	{
		parsed.fullyQualifiedDomainName = config["fullyQualifiedDomainName"].get<std::string>();
	}

	if (config.contains("vmId"))
	{
		parsed.vmId = config["vmId"].get<std::string>();
	}

	if (config.contains("gameServerConnectionInfo"))
	{
		auto connInfoJson = config["gameServerConnectionInfo"];
		parsed.connectionInfo.publicIpV4Address = connInfoJson.value("publicIpV4Adress", ""); // Note: matches typo

		if (connInfoJson.contains("gamePortsConfiguration"))
		{
			for (const auto& portJson : connInfoJson["gamePortsConfiguration"])
			{
				ParsedConfig::GamePort port;
				port.name = portJson.value("name", "");
				port.serverListeningPort = portJson.value("serverListeningPort", 0);
				port.clientConnectionPort = portJson.value("clientConnectionPort", 0);
				parsed.connectionInfo.gamePortsConfiguration.push_back(port);
			}
		}
	}

	return parsed;
}

} // anonymous namespace

class ConfigurationTest : public ::testing::Test
{
protected:
	static constexpr const char* TEST_DIR = "/tmp/gsdk_config_tests";
	std::string configFilePath;

	void SetUp() override
	{
		std::filesystem::create_directories(TEST_DIR);
		configFilePath = std::string(TEST_DIR) + "/testConfig.json";
	}

	void TearDown() override
	{
		if (std::filesystem::exists(configFilePath))
		{
			std::filesystem::remove(configFilePath);
		}
	}

	void WriteConfigToFile(const json& config)
	{
		std::ofstream file(configFilePath);
		file << config.dump(4);
	}

	json ReadConfigFromFile()
	{
		std::ifstream file(configFilePath);
		return json::parse(file);
	}
};

// Mirrors: ConfigAllSetInitializesFine
TEST_F(ConfigurationTest, ConfigAllSetParsesCorrectly)
{
	std::map<std::string, std::string> gameCerts = {
		{"cert1", "thumbprint1"},
		{"cert2", "thumbprint2"}
	};
	std::map<std::string, std::string> metadata = {
		{"key1", "value1"},
		{"key2", "value2"}
	};
	std::map<std::string, std::string> ports = {
		{"port1", "1111"},
		{"port2", "2222"}
	};

	json config = SerializeConfig(
		"testEndpoint", "testServerId", "testLogFolder", "testSharedContentFolder",
		"testCertFolder", gameCerts, metadata, ports
	);

	WriteConfigToFile(config);
	json readback = ReadConfigFromFile();
	ParsedConfig parsed = ParseConfig(readback);

	EXPECT_EQ(parsed.heartbeatEndpoint, "testEndpoint");
	EXPECT_EQ(parsed.serverId, "testServerId");
	EXPECT_EQ(parsed.logFolder, "testLogFolder");
	EXPECT_EQ(parsed.sharedContentFolder, "testSharedContentFolder");
	EXPECT_EQ(parsed.certFolder, "testCertFolder");
	EXPECT_EQ(parsed.gameCerts["cert1"], "thumbprint1");
	EXPECT_EQ(parsed.gameCerts["cert2"], "thumbprint2");
	EXPECT_EQ(parsed.metadata["key1"], "value1");
	EXPECT_EQ(parsed.metadata["key2"], "value2");
	EXPECT_EQ(parsed.ports["port1"], "1111");
	EXPECT_EQ(parsed.ports["port2"], "2222");
}

// Mirrors: LogFolderNotSetInitializesFine
TEST_F(ConfigurationTest, EmptyLogFolderParsesCorrectly)
{
	json config = SerializeConfig(
		"testEndpoint", "testServerId", "", "testSharedContentFolder"
	);

	ParsedConfig parsed = ParseConfig(config);

	EXPECT_EQ(parsed.logFolder, "");
}

// Mirrors: SharedContentFolderNotSetInitializesFine
TEST_F(ConfigurationTest, EmptySharedContentFolderParsesCorrectly)
{
	json config = SerializeConfig(
		"testEndpoint", "testServerId", "testLogFolder", ""
	);

	ParsedConfig parsed = ParseConfig(config);

	EXPECT_EQ(parsed.sharedContentFolder, "");
}

TEST_F(ConfigurationTest, MissingOptionalFieldsDefaultToEmpty)
{
	json config;
	config["heartbeatEndpoint"] = "testEndpoint";
	config["sessionHostId"] = "testServerId";
	config["logFolder"] = "testLogFolder";
	config["sharedContentFolder"] = "testSharedContentFolder";

	ParsedConfig parsed = ParseConfig(config);

	EXPECT_EQ(parsed.heartbeatEndpoint, "testEndpoint");
	EXPECT_EQ(parsed.serverId, "testServerId");
	EXPECT_TRUE(parsed.certFolder.empty());
	EXPECT_TRUE(parsed.gameCerts.empty());
	EXPECT_TRUE(parsed.metadata.empty());
	EXPECT_TRUE(parsed.ports.empty());
	EXPECT_TRUE(parsed.publicIpV4Address.empty());
	EXPECT_TRUE(parsed.fullyQualifiedDomainName.empty());
	EXPECT_TRUE(parsed.vmId.empty());
}

TEST_F(ConfigurationTest, ConnectionInfoParsesCorrectly)
{
	json config = SerializeConfig(
		"testEndpoint", "testServerId", "testLogFolder", "testSharedContentFolder"
	);

	config["gameServerConnectionInfo"] = {
		{"publicIpV4Adress", "1.2.3.4"}, // Note: matches typo in UE code
		{"gamePortsConfiguration", {
			{{"name", "gameport"}, {"serverListeningPort", 7777}, {"clientConnectionPort", 30000}},
			{{"name", "debugport"}, {"serverListeningPort", 8888}, {"clientConnectionPort", 30001}}
		}}
	};

	ParsedConfig parsed = ParseConfig(config);

	EXPECT_EQ(parsed.connectionInfo.publicIpV4Address, "1.2.3.4");
	ASSERT_EQ(parsed.connectionInfo.gamePortsConfiguration.size(), 2u);
	EXPECT_EQ(parsed.connectionInfo.gamePortsConfiguration[0].name, "gameport");
	EXPECT_EQ(parsed.connectionInfo.gamePortsConfiguration[0].serverListeningPort, 7777);
	EXPECT_EQ(parsed.connectionInfo.gamePortsConfiguration[0].clientConnectionPort, 30000);
	EXPECT_EQ(parsed.connectionInfo.gamePortsConfiguration[1].name, "debugport");
	EXPECT_EQ(parsed.connectionInfo.gamePortsConfiguration[1].serverListeningPort, 8888);
	EXPECT_EQ(parsed.connectionInfo.gamePortsConfiguration[1].clientConnectionPort, 30001);
}

TEST_F(ConfigurationTest, ConfigRoundTripsThroughFile)
{
	json config = SerializeConfig(
		"testEndpoint", "testServerId", "testLogFolder", "testSharedContentFolder",
		"testCertFolder", {{"cert1", "thumbprint1"}}, {{"key1", "value1"}}, {{"port1", "1111"}}
	);

	WriteConfigToFile(config);
	json readback = ReadConfigFromFile();
	ParsedConfig parsed = ParseConfig(readback);

	EXPECT_EQ(parsed.heartbeatEndpoint, "testEndpoint");
	EXPECT_EQ(parsed.serverId, "testServerId");
	EXPECT_EQ(parsed.logFolder, "testLogFolder");
	EXPECT_EQ(parsed.sharedContentFolder, "testSharedContentFolder");
	EXPECT_EQ(parsed.certFolder, "testCertFolder");
	EXPECT_EQ(parsed.gameCerts["cert1"], "thumbprint1");
	EXPECT_EQ(parsed.metadata["key1"], "value1");
	EXPECT_EQ(parsed.ports["port1"], "1111");
}

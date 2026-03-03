// Copyright (C) Microsoft Corporation. All rights reserved.

// Standalone unit tests for the Unreal GSDK heartbeat request/response JSON format.
// These tests validate the JSON contracts used by FGSDKInternal::EncodeHeartbeatRequest
// and FGSDKInternal::DecodeHeartbeatResponse without requiring Unreal Engine.

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

using json = nlohmann::json;

namespace {

// Game states matching GSDKInternal.h
enum class GameState
{
	Invalid,
	Initializing,
	StandingBy,
	Active,
	Terminating,
	Terminated,
	Quarantined
};

const char* GameStateNames[] = {
	"Invalid",
	"Initializing",
	"StandingBy",
	"Active",
	"Terminating",
	"Terminated",
	"Quarantined"
};

// Operations matching GSDKInternal.h
enum class Operation
{
	Invalid,
	Continue,
	GetManifest,
	Quarantine,
	Active,
	Terminate,
	Operation_Count
};

// Initialize operation map matching the Unreal GSDK logic
std::map<std::string, Operation> InitializeOperationMap()
{
	std::map<std::string, Operation> map;
	map["Invalid"] = Operation::Invalid;
	map["Continue"] = Operation::Continue;
	map["GetManifest"] = Operation::GetManifest;
	map["Quarantine"] = Operation::Quarantine;
	map["Active"] = Operation::Active;
	map["Terminate"] = Operation::Terminate;
	map["Operation_Count"] = Operation::Operation_Count;
	return map;
}

// Heartbeat request matching FHeartbeatRequest
struct HeartbeatRequest
{
	GameState currentGameState = GameState::Initializing;
	bool isGameHealthy = true;
	std::vector<std::string> connectedPlayerIds;
};

// Mirrors FGSDKInternal::EncodeHeartbeatRequest
json EncodeHeartbeatRequest(const HeartbeatRequest& request,
	std::function<bool()> healthCheck = nullptr)
{
	json j;

	j["CurrentGameState"] = GameStateNames[static_cast<int>(request.currentGameState)];

	bool isHealthy = request.isGameHealthy;
	if (healthCheck)
	{
		isHealthy = healthCheck();
	}
	j["CurrentGameHealth"] = isHealthy ? "Healthy" : "Unhealthy";

	j["CurrentPlayers"] = json::array();
	for (const auto& playerId : request.connectedPlayerIds)
	{
		json player;
		player["PlayerId"] = playerId;
		j["CurrentPlayers"].push_back(player);
	}

	return j;
}

// Maintenance event matching FMaintenanceEvent
struct MaintenanceEvent
{
	std::string eventId;
	std::string eventType;
	std::string resourceType;
	std::vector<std::string> resources;
	std::string eventStatus;
	std::string notBefore;
	std::string description;
	std::string eventSource;
	int durationInSeconds = 0;
};

// Maintenance schedule matching FMaintenanceSchedule
struct MaintenanceSchedule
{
	std::string documentIncarnation;
	std::vector<MaintenanceEvent> events;
};

// Parsed heartbeat response
struct ParsedHeartbeatResponse
{
	std::string operation;
	std::string sessionId;
	std::string sessionCookie;
	std::vector<std::string> initialPlayers;
	std::map<std::string, std::string> configSettings;
	std::string maintenanceUtc;
	MaintenanceSchedule maintenanceSchedule;
	int nextHeartbeatIntervalMs = 0;
	bool hasMaintenanceV1 = false;
	bool hasMaintenanceV2 = false;
	bool operationParseError = false;
};

// Mirrors FGSDKInternal::DecodeHeartbeatResponse
ParsedHeartbeatResponse DecodeHeartbeatResponse(const std::string& responseJson)
{
	ParsedHeartbeatResponse result;

	json j;
	try
	{
		j = json::parse(responseJson);
	}
	catch (...)
	{
		result.operationParseError = true;
		return result;
	}

	// Parse session config
	if (j.contains("sessionConfig"))
	{
		auto sessionConfig = j["sessionConfig"];

		// Parse initial players (only first time)
		if (result.initialPlayers.empty() && sessionConfig.contains("initialPlayers"))
		{
			for (const auto& player : sessionConfig["initialPlayers"])
			{
				result.initialPlayers.push_back(player.get<std::string>());
			}
		}

		// Parse string values from session config into config settings
		for (const auto& [key, value] : sessionConfig.items())
		{
			if (value.is_string())
			{
				result.configSettings[key] = value.get<std::string>();
			}
		}

		// Parse metadata
		if (sessionConfig.contains("metadata"))
		{
			for (const auto& [key, value] : sessionConfig["metadata"].items())
			{
				if (value.is_string())
				{
					result.configSettings[key] = value.get<std::string>();
				}
			}
		}
	}

	// Extract common session config values
	if (result.configSettings.count("sessionId"))
	{
		result.sessionId = result.configSettings["sessionId"];
	}
	if (result.configSettings.count("sessionCookie"))
	{
		result.sessionCookie = result.configSettings["sessionCookie"];
	}

	// Parse maintenance V1
	if (j.contains("nextScheduledMaintenanceUtc"))
	{
		result.hasMaintenanceV1 = true;
		result.maintenanceUtc = j["nextScheduledMaintenanceUtc"].get<std::string>();
	}

	// Parse maintenance V2
	if (j.contains("maintenanceSchedule"))
	{
		result.hasMaintenanceV2 = true;
		auto scheduleJson = j["maintenanceSchedule"];
		result.maintenanceSchedule.documentIncarnation =
			scheduleJson["DocumentIncarnation"].get<std::string>();

		for (const auto& eventJson : scheduleJson["Events"])
		{
			MaintenanceEvent event;
			event.eventId = eventJson["EventId"].get<std::string>();
			event.eventType = eventJson["EventType"].get<std::string>();
			event.resourceType = eventJson["ResourceType"].get<std::string>();
			for (const auto& resource : eventJson["Resources"])
			{
				event.resources.push_back(resource.get<std::string>());
			}
			event.eventStatus = eventJson["EventStatus"].get<std::string>();
			event.notBefore = eventJson["NotBefore"].get<std::string>();
			event.description = eventJson["Description"].get<std::string>();
			event.eventSource = eventJson["EventSource"].get<std::string>();
			event.durationInSeconds = eventJson["DurationInSeconds"].get<int>();
			result.maintenanceSchedule.events.push_back(event);
		}
	}

	// Parse operation
	if (j.contains("operation"))
	{
		try
		{
			result.operation = j["operation"].get<std::string>();
		}
		catch (...)
		{
			result.operationParseError = true;
		}
	}

	// Parse heartbeat interval
	if (j.contains("nextHeartbeatIntervalMs"))
	{
		result.nextHeartbeatIntervalMs = j["nextHeartbeatIntervalMs"].get<int>();
	}

	return result;
}

// Operation resolver matching the Unreal GSDK logic
struct OperationResult
{
	GameState newState;
	bool shutdownCalled = false;
	bool serverActiveCalled = false;
	bool wasValid = true;
	bool wasError = false;
};

OperationResult ResolveOperation(
	const std::string& operationStr,
	GameState currentState,
	const std::map<std::string, Operation>& operationMap)
{
	OperationResult result;
	result.newState = currentState;

	if (operationMap.find(operationStr) == operationMap.end())
	{
		result.wasError = true;
		return result;
	}

	Operation nextOperation = operationMap.at(operationStr);

	switch (nextOperation)
	{
	case Operation::Continue:
		break;
	case Operation::Active:
		if (currentState != GameState::Active)
		{
			result.newState = GameState::Active;
			result.serverActiveCalled = true;
		}
		break;
	case Operation::Terminate:
		if (currentState != GameState::Terminating)
		{
			result.newState = GameState::Terminating;
			result.shutdownCalled = true;
		}
		break;
	default:
		result.wasValid = false;
		break;
	}

	return result;
}

} // anonymous namespace

// Heartbeat Request Tests

class HeartbeatRequestTest : public ::testing::Test {};

// Mirrors: EncodeGameStateAsValidJson (initial state)
TEST_F(HeartbeatRequestTest, EncodeInitialState)
{
	HeartbeatRequest request; // Default: Initializing, healthy, no players

	json encoded = EncodeHeartbeatRequest(request);

	EXPECT_EQ(encoded["CurrentGameState"], "Initializing");
	EXPECT_EQ(encoded["CurrentGameHealth"], "Healthy");
	EXPECT_TRUE(encoded["CurrentPlayers"].empty());
}

// Mirrors: EncodeGameStateAsValidJson (with health callback and players)
TEST_F(HeartbeatRequestTest, EncodeWithStateChangeAndHealthCallback)
{
	HeartbeatRequest request;
	request.currentGameState = GameState::StandingBy;
	request.connectedPlayerIds = {"player1", "player2"};

	auto healthCheck = []() -> bool { return false; };

	json encoded = EncodeHeartbeatRequest(request, healthCheck);

	EXPECT_EQ(encoded["CurrentGameState"], "StandingBy");
	EXPECT_EQ(encoded["CurrentGameHealth"], "Unhealthy");
	ASSERT_EQ(encoded["CurrentPlayers"].size(), 2u);
	EXPECT_EQ(encoded["CurrentPlayers"][0]["PlayerId"], "player1");
	EXPECT_EQ(encoded["CurrentPlayers"][1]["PlayerId"], "player2");
}

TEST_F(HeartbeatRequestTest, EncodeDefaultHealthWithoutCallback)
{
	HeartbeatRequest request;

	json encoded = EncodeHeartbeatRequest(request);

	EXPECT_EQ(encoded["CurrentGameHealth"], "Healthy");
}

TEST_F(HeartbeatRequestTest, EncodeAllGameStates)
{
	const std::vector<std::pair<GameState, std::string>> states = {
		{GameState::Invalid, "Invalid"},
		{GameState::Initializing, "Initializing"},
		{GameState::StandingBy, "StandingBy"},
		{GameState::Active, "Active"},
		{GameState::Terminating, "Terminating"},
		{GameState::Terminated, "Terminated"},
		{GameState::Quarantined, "Quarantined"}
	};

	for (const auto& [state, name] : states)
	{
		HeartbeatRequest request;
		request.currentGameState = state;

		json encoded = EncodeHeartbeatRequest(request);

		EXPECT_EQ(encoded["CurrentGameState"], name) << "Failed for state: " << name;
	}
}

// Heartbeat Response Tests

class HeartbeatResponseTest : public ::testing::Test
{
protected:
	const std::map<std::string, Operation> operationMap = InitializeOperationMap();
};

// Mirrors: DecodeAgentResponseJsonCorrectly
TEST_F(HeartbeatResponseTest, DecodeActiveWithSessionConfig)
{
	std::string responseJson = R"({
		"operation":"Active",
		"sessionConfig":
		{
			"sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
			"sessionCookie":"OreoCookie"
		},
		"nextScheduledMaintenanceUtc":"2018-04-12T16:58:30.1458776Z",
		"nextHeartbeatIntervalMs":30000
	})";

	auto response = DecodeHeartbeatResponse(responseJson);

	EXPECT_EQ(response.sessionId, "eca7e870-da2e-45f9-bb66-30d89064313a");
	EXPECT_EQ(response.sessionCookie, "OreoCookie");
	EXPECT_TRUE(response.hasMaintenanceV1);
	EXPECT_EQ(response.maintenanceUtc, "2018-04-12T16:58:30.1458776Z");
	EXPECT_EQ(response.operation, "Active");
	EXPECT_EQ(response.nextHeartbeatIntervalMs, 30000);

	OperationResult opResult = ResolveOperation(
		response.operation, GameState::StandingBy, operationMap);
	EXPECT_EQ(opResult.newState, GameState::Active);
	EXPECT_TRUE(opResult.serverActiveCalled);
}

// Mirrors: GameState_MaintV2_CallbackInvoked
TEST_F(HeartbeatResponseTest, DecodeMaintenanceV2Schedule)
{
	std::string responseJson = R"({
		"operation":"Active",
		"sessionConfig":
		{
			"sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
			"sessionCookie":"OreoCookie"
		},
		"maintenanceSchedule":
		{
			"DocumentIncarnation": "IncarnationID",
			"Events":
			[
				{
					"EventId": "eventID",
					"EventType": "Reboot",
					"ResourceType": "VirtualMachine",
					"Resources":
					[
						"resourceName"
					],
					"EventStatus": "Scheduled",
					"NotBefore": "2018-04-12T16:58:30.1458776Z",
					"Description": "eventDescription",
					"EventSource": "Platform",
					"DurationInSeconds": 3600
				}
			]
		},
		"nextHeartbeatIntervalMs":30000
	})";

	auto response = DecodeHeartbeatResponse(responseJson);

	EXPECT_TRUE(response.hasMaintenanceV2);
	EXPECT_EQ(response.maintenanceSchedule.documentIncarnation, "IncarnationID");
	ASSERT_EQ(response.maintenanceSchedule.events.size(), 1u);

	const auto& event = response.maintenanceSchedule.events[0];
	EXPECT_EQ(event.eventId, "eventID");
	EXPECT_EQ(event.eventType, "Reboot");
	EXPECT_EQ(event.resourceType, "VirtualMachine");
	ASSERT_EQ(event.resources.size(), 1u);
	EXPECT_EQ(event.resources[0], "resourceName");
	EXPECT_EQ(event.eventStatus, "Scheduled");
	EXPECT_EQ(event.notBefore, "2018-04-12T16:58:30.1458776Z");
	EXPECT_EQ(event.description, "eventDescription");
	EXPECT_EQ(event.eventSource, "Platform");
	EXPECT_EQ(event.durationInSeconds, 3600);
}

// Mirrors: JsonDoesntCrashWhenInvalidJson
TEST_F(HeartbeatResponseTest, InvalidOperationFormatHandledGracefully)
{
	std::string responseJson = R"({
		"operation": {
			"status": "Active"
		},
		"sessionConfig":
		{
			"sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
			"sessionCookie":"OreoCookie"
		}
	})";

	auto response = DecodeHeartbeatResponse(responseJson);

	EXPECT_TRUE(response.operationParseError);
	EXPECT_TRUE(response.operation.empty());
}

// Mirrors: ReturnInitialPlayerListFromJson
TEST_F(HeartbeatResponseTest, DecodeInitialPlayers)
{
	std::string responseJson = R"({
		"operation":"Active",
		"sessionConfig":
		{
			"sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
			"sessionCookie":"OreoCookie",
			"initialPlayers":
			[
				"player0",
				"player1",
				"player2"
			],
			"randomList":
			[
				"item1",
				"item2"
			]
		}
	})";

	auto response = DecodeHeartbeatResponse(responseJson);

	ASSERT_EQ(response.initialPlayers.size(), 3u);
	EXPECT_EQ(response.initialPlayers[0], "player0");
	EXPECT_EQ(response.initialPlayers[1], "player1");
	EXPECT_EQ(response.initialPlayers[2], "player2");
}

// Mirrors: ReturnSessionMetadataFromJson
TEST_F(HeartbeatResponseTest, DecodeSessionMetadata)
{
	std::string responseJson = R"({
		"operation":"Active",
		"sessionConfig":
		{
			"sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
			"sessionCookie":"OreoCookie",
			"metadata":
			{
				"testKey": "testValue"
			}
		}
	})";

	auto response = DecodeHeartbeatResponse(responseJson);

	EXPECT_EQ(response.configSettings["testKey"], "testValue");
}

// Mirrors: AgentOperationStateChangesHandledCorrectly
TEST_F(HeartbeatResponseTest, OperationStateChangesHandledCorrectly)
{
	// Test Active operation
	OperationResult result = ResolveOperation("Active", GameState::StandingBy, operationMap);
	EXPECT_EQ(result.newState, GameState::Active);
	EXPECT_TRUE(result.serverActiveCalled);
	EXPECT_FALSE(result.shutdownCalled);
	EXPECT_TRUE(result.wasValid);

	// Test unknown operation (not in map)
	result = ResolveOperation("TakeOverTheWorld", GameState::Active, operationMap);
	EXPECT_TRUE(result.wasError);
	EXPECT_EQ(result.newState, GameState::Active); // State unchanged

	// Test known but unhandled operation (GetManifest)
	result = ResolveOperation("GetManifest", GameState::Active, operationMap);
	EXPECT_FALSE(result.wasValid);
	EXPECT_EQ(result.newState, GameState::Active); // State unchanged

	// Test Terminate operation
	result = ResolveOperation("Terminate", GameState::Active, operationMap);
	EXPECT_EQ(result.newState, GameState::Terminating);
	EXPECT_TRUE(result.shutdownCalled);
	EXPECT_TRUE(result.wasValid);

	// Test Continue operation (no state change)
	result = ResolveOperation("Continue", GameState::Active, operationMap);
	EXPECT_EQ(result.newState, GameState::Active);
	EXPECT_FALSE(result.shutdownCalled);
	EXPECT_FALSE(result.serverActiveCalled);
	EXPECT_TRUE(result.wasValid);
}

TEST_F(HeartbeatResponseTest, ActiveOperationDoesNotRetriggerWhenAlreadyActive)
{
	OperationResult result = ResolveOperation("Active", GameState::Active, operationMap);
	EXPECT_EQ(result.newState, GameState::Active);
	EXPECT_FALSE(result.serverActiveCalled);
}

TEST_F(HeartbeatResponseTest, TerminateOperationDoesNotRetriggerWhenAlreadyTerminating)
{
	OperationResult result = ResolveOperation(
		"Terminate", GameState::Terminating, operationMap);
	EXPECT_EQ(result.newState, GameState::Terminating);
	EXPECT_FALSE(result.shutdownCalled);
}

TEST_F(HeartbeatResponseTest, HeartbeatIntervalParsedCorrectly)
{
	std::string responseJson = R"({
		"operation":"Continue",
		"nextHeartbeatIntervalMs":30000
	})";

	auto response = DecodeHeartbeatResponse(responseJson);
	EXPECT_EQ(response.nextHeartbeatIntervalMs, 30000);
}

TEST_F(HeartbeatResponseTest, HeartbeatIntervalEnforcesMinimum)
{
	// Matches FConfigurationBase::GetMinimumHeartbeatInterval
	const int minimumHeartbeatInterval = 1000;

	std::string responseJson = R"({
		"operation":"Continue",
		"nextHeartbeatIntervalMs":500
	})";

	auto response = DecodeHeartbeatResponse(responseJson);

	int effectiveInterval = std::max(minimumHeartbeatInterval,
		response.nextHeartbeatIntervalMs);
	EXPECT_EQ(effectiveInterval, minimumHeartbeatInterval);
}

TEST_F(HeartbeatResponseTest, MissingHeartbeatIntervalDefaultsToZero)
{
	std::string responseJson = R"({
		"operation":"Continue"
	})";

	auto response = DecodeHeartbeatResponse(responseJson);
	EXPECT_EQ(response.nextHeartbeatIntervalMs, 0);
}

TEST_F(HeartbeatResponseTest, EmptySessionConfigParsesWithoutError)
{
	std::string responseJson = R"({
		"operation":"Active",
		"sessionConfig":{}
	})";

	auto response = DecodeHeartbeatResponse(responseJson);

	EXPECT_FALSE(response.operationParseError);
	EXPECT_TRUE(response.initialPlayers.empty());
	EXPECT_TRUE(response.sessionId.empty());
	EXPECT_TRUE(response.sessionCookie.empty());
}

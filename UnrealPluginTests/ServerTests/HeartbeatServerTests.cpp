// Copyright (C) Microsoft Corporation. All rights reserved.

// End-to-end tests of the dedicated server build of the plugin (UE_SERVER), compiled against the UE shim only.
// The plugin's real heartbeat thread talks to a fake agent through the shim's HTTP module, and the test thread plays
// the game thread: work the plugin sends to the game thread only runs when the test pumps it.

#include <chrono>
#include <memory>
#include <mutex>
#include <vector>

#include "CoreMinimal.h"
#include "PlayFabGSDK.h"

namespace
{
	// Behaves like the agent: answers "Active" to every heartbeat once the server is allocated, until the server
	// reports Active, and "Terminate" once asked to shut the server down.
	class FFakeAgent
	{
	public:
		UEShim::FFakeHttpResponse HandleRequest(const FString& Url, const FString& Body)
		{
			if (Url.Contains(TEXT("/gsdkinfo")))
			{
				return { 200, TEXT("{}") };
			}

			FString ReportedState;
			TSharedPtr<FJsonObject> Heartbeat;
			if (FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Body), Heartbeat))
			{
				Heartbeat->TryGetStringField(TEXT("CurrentGameState"), ReportedState);
			}

			std::lock_guard<std::mutex> Lock(Mutex);
			ReportedStates.push_back(ReportedState);
			if (bAllocated)
			{
				++HeartbeatsSinceAllocation;
			}

			const TCHAR* Operation = TEXT("Continue");
			if (bTerminating)
			{
				Operation = TEXT("Terminate");
			}
			else if (bAllocated && ReportedState != TEXT("Active"))
			{
				Operation = TEXT("Active");
			}
			return { 200, FString::Printf(TEXT("{\"operation\":\"%s\",\"nextHeartbeatIntervalMs\":1000}"), Operation) };
		}

		void Allocate()
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			bAllocated = true;
		}

		void Terminate()
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			bTerminating = true;
		}

		bool HasSeenState(const FString& State)
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			for (const FString& Reported : ReportedStates)
			{
				if (Reported == State)
				{
					return true;
				}
			}
			return false;
		}

		FString GetLastReportedState()
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			return ReportedStates.empty() ? FString() : ReportedStates.back();
		}

		int32 GetHeartbeatsSinceAllocation()
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			return HeartbeatsSinceAllocation;
		}

	private:
		std::mutex Mutex;
		std::vector<FString> ReportedStates;
		int32 HeartbeatsSinceAllocation = 0;
		bool bAllocated = false;
		bool bTerminating = false;
	};

	template <typename PredicateType>
	bool WaitUntil(PredicateType&& Predicate, bool bPumpGameThread)
	{
		const auto Deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
		while (std::chrono::steady_clock::now() < Deadline)
		{
			if (bPumpGameThread)
			{
				FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
			}
			if (Predicate())
			{
				return true;
			}
			FPlatformProcess::Sleep(0.005f);
		}
		return false;
	}
}

BEGIN_DEFINE_SPEC(FGSDKServerSpec, "GSDKServer", EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FGSDKServerSpec)

void FGSDKServerSpec::Define()
{
	Describe("Heartbeat", [this]()
		{
			// The module can only be started once per process, so this is a single end-to-end scenario.
			It("BusyGameThreadGetsOneOnServerActiveAfterAllocation", [this]()
				{
					std::shared_ptr<FFakeAgent> Agent = std::make_shared<FFakeAgent>();
					UEShim::SetFakeHttpHandler([Agent](const FString&, const FString& Url, const FString& Body) { return Agent->HandleRequest(Url, Body); });

					FPlatformMisc::SetEnvironmentVar(TEXT("GSDK_CONFIG_FILE"), TEXT(""));
					FPlatformMisc::SetEnvironmentVar(TEXT("HEARTBEAT_ENDPOINT"), TEXT("127.0.0.1:56001"));
					FPlatformMisc::SetEnvironmentVar(TEXT("SESSION_HOST_ID"), TEXT("server-tests"));
					FPlatformMisc::SetEnvironmentVar(TEXT("GSDK_LOG_FOLDER"), *(FPaths::ProjectDir() + TEXT("Logs")));

					// Starts the heartbeat thread, as module startup does in a dedicated server.
					FPlayFabGSDKModule& GSDK = FPlayFabGSDKModule::Get();

					int32 serverActiveCount = 0;
					bool bServerActiveOnGameThread = true;
					GSDK.OnServerActive.BindLambda([&serverActiveCount, &bServerActiveOnGameThread]()
						{
							serverActiveCount++;
							bServerActiveOnGameThread = bServerActiveOnGameThread && IsInGameThread();
						});

					// Never freed by the shim's event pool, so the shutdown task can't outlive it.
					FEvent* shutdownCalled = FPlatformProcess::GetSynchEventFromPool(true);
					GSDK.OnShutdown.BindLambda([shutdownCalled]() { shutdownCalled->Trigger(); });

					GSDK.ReadyForPlayers();
					TestTrue("Verify the agent saw the server standing by.", WaitUntil([&Agent]() { return Agent->HasSeenState(TEXT("StandingBy")); }, true));

					// Allocate the server while the game thread is busy: it doesn't run queued work for several heartbeats.
					Agent->Allocate();
					TestTrue("Verify the agent got heartbeats after allocating the server.", WaitUntil([&Agent]() { return Agent->GetHeartbeatsSinceAllocation() >= 3; }, false));
					TestEqual("Verify the server reported Active without waiting for the game thread.", Agent->GetLastReportedState(), TEXT("Active"));
					TestEqual("Verify OnServerActive did not run before the game thread ran.", serverActiveCount, 0);

					FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
					TestEqual("Verify OnServerActive ran exactly once.", serverActiveCount, 1);
					TestTrue("Verify OnServerActive ran on the game thread.", bServerActiveOnGameThread);

					AddExpectedError(TEXT("Received Termination State"));
					Agent->Terminate();
					TestTrue("Verify our shutdown callback was called.", WaitUntil([shutdownCalled]() { return shutdownCalled->Wait(0u); }, true));

					GSDK.OnServerActive.Unbind();
				});
		});
}

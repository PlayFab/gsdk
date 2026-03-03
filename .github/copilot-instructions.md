# Copilot Instructions — PlayFab Game Server SDK (GSDK)

## Repository Overview

This is a multi-language SDK (C++, C#, Java, Go) plus game engine plugins (Unity, Unreal) for integrating game servers with PlayFab Multiplayer Servers. Each language lives in its own top-level directory with independent build systems. There is no single top-level build.

## Build, Test, and Lint

### C++ (`cpp/`)

```bash
# Windows
cd cpp && testbuild.bat

# Linux
cd cpp && ./testbuild.sh

# Manual (both platforms): creates out/ dir, runs CMake
mkdir out && cd out && cmake .. && cmake --build .
```

- Unit tests: `cpp/unittests/` using Visual Studio CppUnitTest framework
- Solution file: `cpp/GSDK.sln` (Visual Studio)

### C# (`csharp/`)

```bash
# Build
dotnet build csharp/GSDK_CSharp_Standard.sln

# Run all tests
dotnet test csharp/Tests/Tests.csproj

# Run a single test
dotnet test csharp/Tests/Tests.csproj --filter "FullyQualifiedName~TestMethodName"
```

- Test framework: MSTest with Moq and FluentAssertions
- Targets .NET 6.0

### Java (`java/`)

```bash
# Build
cd java && ./gradlew build

# Run all tests
cd java && ./gradlew test

# Run a single test class
cd java && ./gradlew test --tests "com.example.TestClassName"
```

- Gradle build, Java 1.8 source compatibility
- Test framework: JUnit 4.12
- Subprojects: `gameserverSDK`, `gameserverSDKConsoleApp`, `javaTestRunnerGame`

### Unity (`UnityGsdk/`)

Open in Unity Editor. Requires scripting define `ENABLE_PLAYFABSERVER_API`.

### Unreal (`UnrealPlugin/`)

C++ Unreal Engine 5.3+ plugin. Requires dedicated server target type.

## Architecture

### Lifecycle State Machine (all SDKs)

Every SDK implements the same state machine for the game server lifecycle:

```
Initializing → StandingBy → Active → Terminating → Terminated
```

- `start()` / `Start()` — Begins heartbeat loop, enters Initializing state
- `readyForPlayers()` / `ReadyForPlayers()` — **Blocks** until the server transitions to Active (allocated by PlayFab)
- Heartbeats are periodic HTTP POSTs to a local VM Agent; the agent controls heartbeat interval

### Configuration

All SDKs support two config sources (checked in order):
1. JSON config file (path from `GSDK_CONFIG_FILE` environment variable)
2. Environment variables

Configuration keys are consistent across SDKs (e.g., `HeartbeatEndpoint`, `ServerId`, `SessionId`, `LogFolder`).

### Callbacks

All SDKs expose the same callback hooks:
- **ShutdownCallback** — Server is being terminated
- **HealthCallback** — Return health status each heartbeat
- **MaintenanceCallback** — Upcoming scheduled maintenance

### Threading Model

- **C++**: `std::thread` for heartbeat loop, `std::mutex` for state
- **C#**: `Task.Run` async heartbeat, `ManualResetEvent` for blocking
- **Java**: `Runnable` heartbeat thread, `CountDownLatch` + `Semaphore` for signaling
- **Go** (experimental): Goroutines and channels

## Key Conventions

- **Static/singleton API**: All SDKs expose a static class (`GSDK`, `GameserverSDK`, `PlayFabMultiplayerAgentAPI`) — no instantiation needed.
- **Namespaces**: `Microsoft.Azure.Gaming` (C++/C#), `com.microsoft.azure.gaming` (Java).
- **Version format**: `MAJOR.MINOR.YYMMDD[suffix]` — the `updateVersion.js` script increments minor and sets today's date as build number.
- **NuGet/Maven packages**: C++ published as `com.playfab.cppgsdk.v140`, C# as `com.playfab.csharpgsdk`, Java as `com.playfab:gameserverSDK`.
- **Go SDK** (`experimental/go/`) is experimental and not production-ready.

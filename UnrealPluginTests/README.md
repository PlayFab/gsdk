# Unreal GSDK tests without Unreal Engine

This folder builds the Unreal GSDK plugin in [`../UnrealPlugin`](../UnrealPlugin) and its automation tests with a plain C++ compiler, so they run on any machine and in CI (`.github/workflows/unreal-tests.yml`) without installing Unreal Engine. It's only for SDK maintainers. It's kept outside `UnrealPlugin/` so that it isn't copied into game projects along with the plugin.

It compiles, **unmodified**:

- the plugin sources in [`../UnrealPlugin/Source/PlayFabGSDK`](../UnrealPlugin/Source/PlayFabGSDK), and
- the automation spec [`../UnrealPlugin/TestingProject/Source/SlateUGS/Private/Tests/GsdkTests.cpp`](../UnrealPlugin/TestingProject/Source/SlateUGS/Private/Tests/GsdkTests.cpp), the same spec the TestingProject runs in the editor,

against [`Shim/`](Shim), a small emulation of the Unreal Engine APIs the plugin uses: `FString`, `TArray`, `TMap`, shared pointers, `TFunction`/`TUniqueFunction`, delegates, `UE_LOG`, `FEvent`/`FScopeLock`, `Async`/`AsyncTask`, the Json and HTTP modules, the module manager, and the automation spec framework (`BEGIN_DEFINE_SPEC`, `Describe`, `It`, `TestEqual`, `AddExpectedError`, ...).

Two executables are built from the same plugin sources:

| Executable | Plugin build configuration | Tests |
|---|---|---|
| `GSDKAutomationTests` | Editor automation tests (`WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR`) | `GsdkTests.cpp` |
| `GSDKServerTests` | Dedicated server (`UE_SERVER`) | [`ServerTests/`](ServerTests): runs the plugin's real heartbeat thread against a fake agent, with the test thread acting as the game thread |

A third executable, `UEShimTests`, runs [`ShimTests/`](ShimTests): tests of the shim itself, which check that it behaves like Unreal where the plugin's tests rely on it, for example that the Json module rejects the same malformed JSON as Unreal.

## Running the tests

You need CMake 3.20+ and a C++20 compiler (GCC, Clang or MSVC). From the repository root:

```bash
cmake -S UnrealPluginTests -B UnrealPluginTests/build
cmake --build UnrealPluginTests/build --config Release
ctest --test-dir UnrealPluginTests/build -C Release --output-on-failure
```

To run some of the tests, pass filters to an executable. As with `Automation RunTests`, a test runs if its full name contains one of the filters:

```bash
UnrealPluginTests/build/GSDKAutomationTests GSDK.Tests.ActiveResponseBurst
```

## About the shim

The shim is not Unreal Engine. It only emulates what the GSDK uses, and it follows Unreal's behavior where that matters to the plugin:

- `TCHAR` is `wchar_t`, `UE_LOG` and `FString::Printf` require `TEXT()` format strings, `FString ==` and `TMap<FString, ...>` lookups ignore case, `MoveTemp` rejects const objects and rvalues, `TUniqueFunction` is move-only and `FCriticalSection` is recursive.
- `AsyncTask(ENamedThreads::GameThread, ...)` queues work that only runs when the test calls `FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread)`, so a test decides when the "game thread" runs. Work for other threads runs on a background thread, and `Async(EAsyncExecution::Thread, ...)` starts a thread.
- The HTTP module sends requests to an in-process fake server (`UEShim::SetFakeHttpHandler`). Responses are available right away, and completion delegates run on the game thread.
- A test fails if it records an error, if an unexpected error is logged while it runs, or if a message expected with `AddExpectedError`/`AddExpectedMessage` isn't logged the expected number of times. Expected messages are matched case-insensitively, and expected errors also match warnings.
- Reflection markup (`UCLASS`, `UPROPERTY`, ...) is ignored, and UObject/Blueprint features aren't available.

If a test passes here but fails in the editor, the editor is right: fix the shim, and add a test for the fix to [`ShimTests/`](ShimTests). When the plugin starts using a new Unreal API, add it to the shim.

## Running the tests in Unreal Engine

These tests complement the in-editor tests, they don't replace them. To run the spec in the editor you need an Unreal Engine 5.3 or later build (the TestingProject's `EngineAssociation` points to a source build, so switch it to your engine first). On Windows, from the engine's root folder:

```bat
Engine\Build\BatchFiles\Build.bat SlateUGSEditor Win64 Development -Project="<repo>\UnrealPlugin\TestingProject\SlateUGS.uproject" -WaitMutex
Engine\Binaries\Win64\UnrealEditor-Cmd.exe "<repo>\UnrealPlugin\TestingProject\SlateUGS.uproject" -ExecCmds="Automation RunTests GSDK.Tests;Quit" -TestExit="Automation Test Queue Empty" -unattended -nopause -nullrhi -nosplash -log -ReportExportPath="<repo>\UnrealPlugin\TestingProject\Saved\Automation"
```

The spec's tests are named `-GSDK.Tests.<test>`, and the `RunTests` filter matches any part of a test name.

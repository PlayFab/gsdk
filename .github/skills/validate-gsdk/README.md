# validate-gsdk

An [Agent Skill](https://agentskills.io) that validates your PlayFab GSDK integration. Works with GitHub Copilot and Claude Code.

## What it does

- Auto-detects which GSDK language you're using (C++, C#, Java, Unity, Unreal)
- Validates lifecycle ordering, callback registration, threading, and dependencies
- Reports issues by severity with file locations and suggested fixes
- Asks whether you want fixes applied or presented for review

## Installation

Copy the entire `validate-gsdk/` directory into your project:

### GitHub Copilot

```bash
# From your game server project root
mkdir -p .github/skills
cp -r path/to/gsdk/.github/skills/validate-gsdk .github/skills/
```

Then use the skill by name in Copilot Chat:
```
Use the /validate-gsdk skill to check my GSDK integration.
```

### Claude Code

```bash
# From your game server project root
mkdir -p .claude/skills
cp -r path/to/gsdk/.github/skills/validate-gsdk .claude/skills/
```

Then invoke in Claude Code:
```
Use the /validate-gsdk skill to check my GSDK integration.
```

## Example Output

Here's what the skill reports when run against a C# sample with deliberate mistakes (missing callbacks, blocking main thread, deprecated APIs):

```
Detected SDK: C# (found com.playfab.csharpgsdk in BrokenSample.csproj)

=== Validation Results ===

[Error] U3 — No shutdown callback registered
  File: Program.cs (line 47-51)
  Fix: Add GameserverSDK.RegisterShutdownCallback(OnShutdown) after Start()

[Error] U4 — No health callback registered
  File: Program.cs (line 47-51)
  Fix: Add GameserverSDK.RegisterHealthCallback(IsHealthy) after Start()

[Warning] U5 — ReadyForPlayers() blocks the main thread
  File: Program.cs (line 60)
  Fix: Wrap in Task.Run(() => GameserverSDK.ReadyForPlayers())

[Warning] CS2 — ReadyForPlayers() not on a background thread
  File: Program.cs (line 60)
  Fix: Use Task.Run or a dedicated Thread to avoid blocking Main()

[Warning] U6 — UpdateConnectedPlayers() is never called
  File: Program.cs
  Fix: Call GameserverSDK.UpdateConnectedPlayers(players) when players join/leave

[Warning] U7 — SessionCookie accessed without null-check after allocation
  File: Program.cs (line 65)
  Fix: Use TryGetValue for session-specific config keys

[Info] U8 — Using deprecated RegisterMaintenanceCallback
  File: Program.cs (line 51)
  Fix: Migrate to GameserverSDK.RegisterMaintenanceV2Callback(Action<MaintenanceSchedule>)

Found 2 errors, 4 warnings, 1 info.

Would you like me to apply these fixes to your code, or just present the diffs for review?
```

## Supported SDKs

| SDK | Detection | Key checks |
|---|---|---|
| C++ | CMakeLists.txt / vcxproj with cppsdk | Library linkage, thread safety in callbacks |
| C# | .csproj with NuGet package | Task.Run for ReadyForPlayers, .NET 6.0+ |
| Java | build.gradle / pom.xml with com.playfab | Dedicated thread, exception handling |
| Unity | ProjectSettings + PlayFabMultiplayerAgentAPI | Scripting defines, coroutine heartbeats |
| Unreal | .uproject with PlayFabGSDK plugin | Server target, delegate binding order |

---
name: validate-gsdk
description: Validates PlayFab Game Server SDK (GSDK) integrations in game server projects. Use when asked to check, validate, audit, or fix a GSDK implementation. Supports C++, C#, Java, Unity, and Unreal SDKs.
---

# Validate GSDK Integration

You are a PlayFab GSDK integration validator. Scan the developer's project, detect which SDK language they use, validate their integration against known rules, and help fix any issues.

## Step 1: Detect SDK Language

Search the project for these markers (stop at first match):

| Marker | SDK | Reference |
|---|---|---|
| `*.uproject` with PlayFabGSDK in Plugins | Unreal | [references/unreal.md](references/unreal.md) |
| `ProjectSettings/` + `PlayFabMultiplayerAgentAPI` usage | Unity | [references/unity.md](references/unity.md) |
| `CMakeLists.txt` or `*.vcxproj` referencing gsdk/cppsdk | C++ | [references/cpp.md](references/cpp.md) |
| `*.csproj` with `com.playfab.csharpgsdk` | C# | [references/csharp.md](references/csharp.md) |
| `build.gradle` or `pom.xml` with `com.playfab` | Java | [references/java.md](references/java.md) |

If no GSDK usage found, tell the user and ask if they need help adding it.

Once detected, load the corresponding reference file for language-specific rules.

## Step 2: Validate Universal Rules

These apply to **all** GSDK languages:

| ID | Rule | Severity |
|---|---|---|
| U1 | `start()` / `Start()` is called before other GSDK methods | Error |
| U2 | `readyForPlayers()` / `ReadyForPlayers()` is called | Error |
| U3 | A shutdown callback is registered | Error |
| U4 | A health callback is registered | Warning |
| U5 | `readyForPlayers()` does not block the main/UI thread | Warning |
| U6 | `updateConnectedPlayers()` is called when players join/leave | Warning |
| U7 | Session-only config keys (`SessionId`, `SessionCookie`) not read before allocation | Warning |
| U8 | If `MaintenanceCallback` is used, suggest `MaintenanceV2Callback` | Info |

Then check the language-specific rules from the loaded reference file.

## Step 3: Report

For each violated rule, output:

```
[Severity] RuleID — Description
  File: path/to/file (line N)
  Fix: what to change
```

Group by severity: Error → Warning → Info. If no issues found, confirm the integration looks correct.

## Step 4: Ask User

After reporting, ask: **"Would you like me to apply these fixes to your code, or just present the diffs for review?"**

- If they want fixes applied: make minimal, targeted changes.
- If they want review: show the proposed diff for each fix without modifying files.

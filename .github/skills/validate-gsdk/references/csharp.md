# C# GSDK Rules

## Required Setup
- NuGet package: `com.playfab.csharpgsdk` referenced in a `.csproj` file
- Target framework: .NET 6.0 or later

## API Pattern
```csharp
// Correct order
GameserverSDK.RegisterShutdownCallback(OnShutdown);
GameserverSDK.RegisterHealthCallback(OnHealth);
GameserverSDK.Start();

// BLOCKS — must not be on the main async context
Task.Run(() => {
    bool allocated = GameserverSDK.ReadyForPlayers();
});
```

## Language-Specific Rules

| ID | Rule | Severity |
|---|---|---|
| CS1 | `com.playfab.csharpgsdk` is in a `.csproj` PackageReference or `packages.config` | Error |
| CS2 | `ReadyForPlayers()` is wrapped in `Task.Run` or called on a background thread | Warning |
| CS3 | Target framework is `net6.0` or later | Info |
| CS4 | Health callback does not throw exceptions (they are silently swallowed, server marked unhealthy) | Warning |

## Common Mistakes
- Calling `ReadyForPlayers()` with `await` on the main async context — it uses `ManualResetEvent.WaitOne()` internally, which blocks the thread and can deadlock
- Health callback throwing an exception — the SDK catches it silently and reports the server as unhealthy
- Missing the NuGet package — build error `The type or namespace name 'GameserverSDK' could not be found`

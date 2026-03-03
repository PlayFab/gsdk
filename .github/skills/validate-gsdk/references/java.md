# Java GSDK Rules

## Required Setup
- Gradle: `com.playfab:gameserverSDK` dependency in `build.gradle`
- Maven: `com.playfab:gameserverSDK` dependency in `pom.xml`
- Java 1.8+ source compatibility

## API Pattern
```java
// Correct order
GameserverSDK.registerShutdownCallback(() -> onShutdown());
GameserverSDK.registerHealthCallback(() -> GameHostHealth.Healthy);

// start() is auto-called by the static initializer, but explicit call is fine
GameserverSDK.start();

// BLOCKS via CountDownLatch — call in a dedicated thread
new Thread(() -> {
    try {
        boolean allocated = GameserverSDK.readyForPlayers();
    } catch (GameserverSDKInitializationException e) {
        // Handle missing heartbeat endpoint or server ID
    }
}).start();
```

## Language-Specific Rules

| ID | Rule | Severity |
|---|---|---|
| JV1 | `com.playfab:gameserverSDK` dependency in build.gradle or pom.xml | Error |
| JV2 | `readyForPlayers()` is called in a dedicated thread (it blocks via CountDownLatch) | Warning |
| JV3 | `GameserverSDKInitializationException` is caught around `readyForPlayers()` | Warning |
| JV4 | Health callback returns a `GameHostHealth` enum value, not a boolean | Warning |

## Common Mistakes
- Calling `readyForPlayers()` on the main thread — blocks indefinitely until allocation
- Not catching `GameserverSDKInitializationException` — crashes if `HeartbeatEndpoint` or `ServerId` env vars are missing
- Health callback returning wrong type — Java SDK expects `Supplier<GameHostHealth>`, not `Supplier<Boolean>`

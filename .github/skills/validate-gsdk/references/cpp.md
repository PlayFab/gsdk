# C++ GSDK Rules

## Required Setup
- `#include "gsdk.h"` in the server's main source file
- Project links against the cppsdk library (check CMakeLists.txt `add_subdirectory` or `target_link_libraries`, or vcxproj references)

## API Pattern
```cpp
// Correct order
GSDK::registerShutdownCallback(onShutdown);
GSDK::registerHealthCallback(onHealth);
GSDK::start();                      // starts heartbeat thread
bool allocated = GSDK::readyForPlayers(); // BLOCKS until Active or Terminating
```

## Language-Specific Rules

| ID | Rule | Severity |
|---|---|---|
| CP1 | `#include "gsdk.h"` is present in source files calling GSDK | Error |
| CP2 | Project links against cppsdk (CMakeLists.txt or vcxproj) | Error |
| CP3 | Shutdown/health callbacks don't access shared mutable state without `std::mutex` | Warning |
| CP4 | `readyForPlayers()` is not called on the main thread if the main thread runs game logic | Warning |

## Common Mistakes
- Forgetting to link cppsdk in CMakeLists.txt — causes unresolved symbol errors at link time
- Calling `readyForPlayers()` on the main thread — blocks all game logic until allocation
- Accessing `SESSION_COOKIE_KEY` or `SESSION_ID_KEY` from `getConfigSettings()` before the server is in Active state — returns empty string

cmake_minimum_required(VERSION 3.14)
project(UnrealGSDKTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest json)

enable_testing()

add_executable(unreal_gsdk_tests
  GSDKConfigTests.cpp
  GSDKHeartbeatTests.cpp
)

target_link_libraries(unreal_gsdk_tests
  GTest::gtest_main
  nlohmann_json::nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(unreal_gsdk_tests)

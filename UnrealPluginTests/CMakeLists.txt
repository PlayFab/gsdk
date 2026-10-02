# Copyright (C) Microsoft Corporation. All rights reserved.
#
# Builds the Unreal GSDK plugin sources and its automation tests against a small Unreal Engine API shim, so they run
# without Unreal Engine (see README.md). Two configurations of the same plugin sources are built:
#   GSDKAutomationTests - the editor automation test build (WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR), running the
#                         spec in UnrealPlugin/TestingProject/Source/SlateUGS/Private/Tests/GsdkTests.cpp unmodified.
#   GSDKServerTests     - the dedicated server build (UE_SERVER), running end-to-end tests in ServerTests/ that drive
#                         the real heartbeat thread against a fake agent.

cmake_minimum_required(VERSION 3.20)
project(PlayFabGSDKUnrealTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
enable_testing()

set(UNREAL_PLUGIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../UnrealPlugin")
set(PLUGIN_MODULE_DIR "${UNREAL_PLUGIN_DIR}/Source/PlayFabGSDK")
set(TESTING_PROJECT_TESTS_DIR "${UNREAL_PLUGIN_DIR}/TestingProject/Source/SlateUGS/Private/Tests")

if(MSVC)
    set(UE_SHIM_COMPILE_OPTIONS /W3 /permissive- /Zc:__cplusplus /utf-8 /EHsc /MP)
else()
    set(UE_SHIM_COMPILE_OPTIONS -Wall -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable)
endif()

add_library(UEShim STATIC
    Shim/Private/Automation.cpp
    Shim/Private/Core.cpp
    Shim/Private/Http.cpp
    Shim/Private/Json.cpp)
target_include_directories(UEShim PUBLIC Shim/Include)
target_compile_options(UEShim PRIVATE ${UE_SHIM_COMPILE_OPTIONS})
target_link_libraries(UEShim PUBLIC Threads::Threads)

set(PLUGIN_SOURCES
    "${PLUGIN_MODULE_DIR}/Private/GSDKConfiguration.cpp"
    "${PLUGIN_MODULE_DIR}/Private/GSDKInternal.cpp"
    "${PLUGIN_MODULE_DIR}/Private/GSDKInternalUtils.cpp"
    "${PLUGIN_MODULE_DIR}/Private/GSDKUtils.cpp"
    "${PLUGIN_MODULE_DIR}/Private/PlayFabGSDK.cpp")

function(add_gsdk_test_executable TARGET_NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEFINITIONS" ${ARGN})
    add_executable(${TARGET_NAME} ${PLUGIN_SOURCES} ${ARG_SOURCES} Runner/TestMain.cpp)
    target_include_directories(${TARGET_NAME} PRIVATE
        "${PLUGIN_MODULE_DIR}/Public"
        "${PLUGIN_MODULE_DIR}/Private"
        "${UNREAL_PLUGIN_DIR}/Source")
    target_compile_definitions(${TARGET_NAME} PRIVATE PLAYFABGSDK_API= ${ARG_DEFINITIONS})
    target_compile_options(${TARGET_NAME} PRIVATE ${UE_SHIM_COMPILE_OPTIONS})
    target_link_libraries(${TARGET_NAME} PRIVATE UEShim)
endfunction()

add_gsdk_test_executable(GSDKAutomationTests
    SOURCES
        "${TESTING_PROJECT_TESTS_DIR}/GsdkTests.cpp"
        "${TESTING_PROJECT_TESTS_DIR}/TestConfiguration.cpp"
    DEFINITIONS WITH_DEV_AUTOMATION_TESTS=1 WITH_EDITOR=1 UE_SERVER=0)

add_gsdk_test_executable(GSDKServerTests
    SOURCES ServerTests/HeartbeatServerTests.cpp
    DEFINITIONS WITH_DEV_AUTOMATION_TESTS=0 WITH_EDITOR=0 UE_SERVER=1)

add_test(NAME GSDKAutomationTests COMMAND GSDKAutomationTests)
add_test(NAME GSDKServerTests COMMAND GSDKServerTests)
set_tests_properties(GSDKAutomationTests GSDKServerTests PROPERTIES TIMEOUT 120)

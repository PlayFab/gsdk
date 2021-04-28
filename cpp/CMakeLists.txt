cmake_minimum_required(VERSION 3.2)
project(GSDK_CPP)

# if no build type specified, default to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# if we're building for Windows, use the included CURL lib
if(WIN32)
    # TODO: handle something that isn't Debug/Release when pathing this out
    set(CURL_LIBRARY     "dependencies/libcurl-vc15-x64-${CMAKE_BUILD_TYPE}-dll-ssl-dll-ipv6-sspi/lib/")
    set(CURL_INCLUDE_DIR "dependencies/libcurl-vc15-x64-${CMAKE_BUILD_TYPE}-dll-ssl-dll-ipv6-sspi/include/")
endif()

# if we're Windows, the above handles the requirement
# if we're Linux, install libcurl via package manager or vcpkg
find_package(CURL REQUIRED)

add_library(GSDK_CPP
    "cppsdk/gsdk.cpp"
    "cppsdk/gsdkConfig.cpp"
    "cppsdk/gsdkLog.cpp"
    "cppsdk/gsdkUtils.cpp"
    "cppsdk/jsoncpp.cpp"
    "cppsdk/ManualResetEvent.cpp"

    # TODO: Do we need these??
    "cppsdk/source/playfab/PlayFabAdminApi.cpp"
    "cppsdk/source/playfab/PlayFabClientApi.cpp"
    "cppsdk/source/playfab/PlayFabEntityApi.cpp"
    "cppsdk/source/playfab/PlayFabError.cpp"
    "cppsdk/source/playfab/PlayFabHttp.cpp"
    "cppsdk/source/playfab/PlayFabMatchmakerApi.cpp"
    "cppsdk/source/playfab/PlayFabServerApi.cpp"
    "cppsdk/source/playfab/PlayFabSettings.cpp"
)

target_include_directories(GSDK_CPP PRIVATE
    cppsdk
    cppsdk/include)

set_target_properties(GSDK_CPP PROPERTIES CXX_STANDARD 14)

# set output names and other flags
if(UNIX)
    set_target_properties(GSDK_CPP PROPERTIES OUTPUT_NAME "GSDK_CPP_Linux")
    target_compile_options(GSDK_CPP PRIVATE -DGSDK_LINUX)
elseif(WIN32)
    set_target_properties(GSDK_CPP PROPERTIES OUTPUT_NAME "GSDK_CPP_Windows")
    target_include_directories(GSDK_CPP PRIVATE "dependencies/libcurl-vc15-x64-${CMAKE_BUILD_TYPE}-dll-ssl-dll-ipv6-sspi/include/")
    target_compile_options(GSDK_CPP PRIVATE -DGSDK_WINDOWS)
endif()

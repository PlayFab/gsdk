cmake_minimum_required(VERSION 3.2)

add_library(GSDK_CPP_Linux
    cpp/cppsdk/gsdk.cpp
    cpp/cppsdk/gsdkConfig.cpp
    cpp/cppsdk/gsdkLog.cpp
    cpp/cppsdk/gsdkUtils.cpp
    cpp/cppsdk/jsoncpp.cpp
    cpp/cppsdk/ManualResetEvent.cpp
)

target_compile_options(GSDK_CPP_Linux PRIVATE -DGSDK_LINUX)
set_target_properties(GSDK_CPP_Linux PROPERTIES CXX_STANDARD 14)
target_include_directories(GSDK_CPP_Linux PRIVATE "cpp/cppsdk")

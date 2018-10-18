
// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include <list>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <cstring>
#include <stdio.h>
#include <future>

#ifdef GSDK_WINDOWS
#include "windows.h"
#define CURL_GLOBAL_GSDK_INIT_FLAGS CURL_GLOBAL_WIN32
#endif

#ifdef GSDK_LINUX
typedef unsigned int HRESULT;
#define SUCCEEDED(hr)   (((HRESULT)(hr)) >= 0)
#define FAILED(hr)      (((HRESULT)(hr)) < 0)
#define S_OK            ((HRESULT)0L)
#define CURL_GLOBAL_GSDK_INIT_FLAGS CURL_GLOBAL_NOTHING
#endif

#define WIDE2(x) L##x
#define WIDECHAR(x) WIDE2(x)
#define __WFUNCTION__ WIDECHAR(__func__)

inline std::string WSTR2STR(const std::wstring &wstr) { return std::string(wstr.begin(), wstr.end()); }
inline std::string WCHAR2STR(const wchar_t *wcharPtr) { std::wstring wstr(wcharPtr); return std::string(wstr.begin(), wstr.end()); }
inline std::wstring STR2WSTR(const std::string &str) { return std::wstring(str.begin(), str.end()); }
inline std::wstring CHAR2WSTR(const char *charPtr) { std::string str(charPtr); return std::wstring(str.begin(), str.end()); }
#define WSTR2CHAR( wstr ) WSTR2STR( wstr ).c_str()
#define WSTR2WCHAR( wstr ) wstr.c_str()
#define WCHAR2CHAR( wcharPtr ) WCHAR2STR( wcharPtr ).c_str()
#define STR2CHAR( str ) str.c_str()
#define STR2WCHAR( str ) STR2WSTR( str ).c_str()
#define CHAR2WCHAR( charPtr ) CHAR2WSTR( charPtr ).c_str()

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {

            class cGSDKUtils
            {
            public:
                static std::string getEnvironmentVariable(const char *environmentVariableName);
                static std::wstring getEnvironmentVariableW(const wchar_t *environmentVariableName);
                static bool createDirectoryIfNotExists(std::string path);
                static time_t tm2timet_utc(struct tm *tm);
            };

        }
    }
}

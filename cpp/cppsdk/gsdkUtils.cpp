
// Copyright (C) Microsoft Corporation. All rights reserved.

#include "gsdkCommonPch.h"
#include "gsdkInternal.h"

#ifdef GSDK_WINDOWS
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "experimental/filesystem"
#include "process.h"
#endif

#ifdef GSDK_LINUX
#include "sys/stat.h"
#include "sys/types.h"
#include "unistd.h"
#endif

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {

            std::string cGSDKUtils::getEnvironmentVariable(const char *environmentVariableName)
            {
                #ifdef GSDK_LINUX
                char* envVal = ::getenv(environmentVariableName);
                if (envVal != nullptr)
                {
                    return envVal;
                }
                else
                {
                    return "";
                }
                #else
                size_t logFolderNameLength = 0;
                char logFolderBuffer[8192]; // significantly arbitrarily larger than any path we will ever create
                if (::getenv_s(&logFolderNameLength, logFolderBuffer, environmentVariableName) == 0)
                {
                    return logFolderBuffer;
                }
                else
                {
                    return "";
                }
                #endif
            }

            std::wstring cGSDKUtils::getEnvironmentVariableW(const wchar_t *environmentVariableName)
            {
                return STR2WSTR(cGSDKUtils::getEnvironmentVariable(WCHAR2CHAR(environmentVariableName)));
            }

            bool cGSDKUtils::createDirectoryIfNotExists(std::string path)
            {
                try
                {
                    #ifdef GSDK_LINUX
                    return mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH | S_IXOTH);
                    #else
                    std::experimental::filesystem::create_directories(std::experimental::filesystem::path(path));
                    return true;
                    #endif
                }
                catch (...)
                {
                    return false;
                }
            }

            time_t cGSDKUtils::tm2timet_utc(tm * tm)
            {
                #ifdef GSDK_LINUX
                return timegm(tm);
                #else
                return _mkgmtime(tm);
                #endif
            }

        }
    }
}

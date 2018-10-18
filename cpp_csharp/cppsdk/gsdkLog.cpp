
// Copyright (C) Microsoft Corporation. All rights reserved.

#include "gsdkCommonPch.h"
#include "gsdkInternal.h"

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {

            GSDKLogMethod::GSDKLogMethod(const char *methodName)
            {
                m_hr = S_OK;
                m_methodName = methodName;
                GSDK::logMessage(" - GSDKMethodEntry: " + m_methodName);
            }

            GSDKLogMethod::~GSDKLogMethod()
            {
                std::string msg = " - GSDKMethodEntry: " + m_methodName + " Result: " + std::to_string(m_hr);
                if (!m_exception_message.empty())
                    msg += " Exception: " + m_exception_message;
                GSDK::logMessage(msg);
            }

            void GSDKLogMethod::setExceptionInformation(const std::exception &ex)
            {
                m_exception_message = ex.what();
            }

            HRESULT GSDKLogMethod::setHResult(HRESULT hr)
            {
                m_hr = hr;
                return hr;
            }

        }
    }
}

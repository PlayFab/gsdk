#pragma once

#include "..\cppsdk\gsdkConfig.h"

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {
            class TestConfig : public Configuration
            {
            public:
                TestConfig( const std::string & heartbeatEndpoint,
                            const std::string & serverId,
                            const std::string & logFolder,
                            const std::string & certFolder = std::string(),
                            const std::unordered_map<std::string, std::string> & gameCerts = std::unordered_map<std::string, std::string>(),
                            const std::string & titleId = std::string(),
                            const std::string & buildId = std::string(),
                            const std::string & region = std::string(),
                            const std::unordered_map<std::string, std::string> & metadata = std::unordered_map<std::string, std::string>());

                const std::string &getHeartbeatEndpoint();
                const std::string &getServerId();
                const std::string &getLogFolder();
                const std::string &getCertificateFolder();
                const std::unordered_map<std::string, std::string> &getGameCertificates();
                const std::string &getTitleId();
                const std::string &getBuildId();
                const std::string &getRegion();
                const std::unordered_map<std::string, std::string> &getBuildMetadata();
                bool shouldLog();
                bool shouldHeartbeat();

            private:
                std::string m_heartbeatEndpoint;
                std::string m_serverId;
                std::string m_logFolder;
                std::string m_certFolder;
                std::unordered_map<std::string, std::string> m_gameCerts;
                std::unordered_map<std::string, std::string> m_metadata;
                std::string m_titleId;
                std::string m_buildId;
                std::string m_region;
            };
        }
    }
}
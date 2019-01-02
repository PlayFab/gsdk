#pragma once
#include <unordered_map>

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {
            class Configuration
            {
            public:
                virtual const std::string &getHeartbeatEndpoint() = 0;
                virtual const std::string &getServerId() = 0;
                virtual const std::string &getLogFolder() = 0;
                virtual const std::string &getCertificateFolder() = 0;
				virtual const std::string &getSharedContentFolder() = 0;
                virtual const std::unordered_map<std::string, std::string> &getGameCertificates() = 0;
                virtual const std::string &getTitleId() = 0;
                virtual const std::string &getBuildId() = 0;
                virtual const std::string &getRegion() = 0;
                virtual const std::unordered_map<std::string, std::string> &getBuildMetadata() = 0;
                virtual const std::unordered_map<std::string, std::string> &getGamePorts() = 0;
                virtual bool shouldLog() = 0;
                virtual bool shouldHeartbeat() = 0;

            protected:
                static constexpr const char* HEARTBEAT_ENDPOINT_ENV_VAR = "HEARTBEAT_ENDPOINT";
                static constexpr const char* SERVER_ID_ENV_VAR = "SESSION_HOST_ID";
                static constexpr const char* LOG_FOLDER_ENV_VAR = "GSDK_LOG_FOLDER";
                static constexpr const char* TITLE_ID_ENV_VAR = "PF_TITLE_ID";
                static constexpr const char* BUILD_ID_ENV_VAR = "PF_BUILD_ID";
                static constexpr const char* REGION_ENV_VAR = "PF_REGION";
                static constexpr const char* SHARED_CONTENT_FOLDER_ENV_VAR = "SHARED_CONTENT_FOLDER";
            };

            class ConfigurationBase : public Configuration
            {
            public:
                ConfigurationBase();

                const std::string &getTitleId();
                const std::string &getBuildId();
                const std::string &getRegion();
                bool shouldLog();
                bool shouldHeartbeat();

            private:
                // These are set in the base class
                std::string m_titleId;
                std::string m_buildId;
                std::string m_region;
            };

            class EnvironmentVariableConfiguration : public ConfigurationBase
            {
            public:
                EnvironmentVariableConfiguration();

                const std::string &getHeartbeatEndpoint();
				const std::string &getServerId();
                const std::string &getLogFolder();
                const std::string &getCertificateFolder();
				const std::string &getSharedContentFolder();
                const std::unordered_map<std::string, std::string> &getGameCertificates();
                const std::unordered_map<std::string, std::string> &getBuildMetadata();
                const std::unordered_map<std::string, std::string> &getGamePorts();

            private:
                std::string m_heartbeatEndpoint;
                std::string m_serverId;
                std::string m_logFolder;
                std::string m_certFolder;
				std::string m_sharedContentFolder;
                std::unordered_map<std::string, std::string> m_gameCerts;
                std::unordered_map<std::string, std::string> m_metadata;
                std::unordered_map<std::string, std::string> m_ports;
            };

            class JsonFileConfiguration : public ConfigurationBase
            {
            public:
                JsonFileConfiguration(const std::string &file_name);

                const std::string &getHeartbeatEndpoint();
                const std::string &getServerId();
                const std::string &getLogFolder();
                const std::string &getCertificateFolder();
				const std::string &getSharedContentFolder();
                const std::unordered_map<std::string, std::string> &getGameCertificates();
                const std::unordered_map<std::string, std::string> &getBuildMetadata();
                const std::unordered_map<std::string, std::string> &getGamePorts();

            private:
                std::string m_heartbeatEndpoint;
                std::string m_serverId;
                std::string m_logFolder;
                std::string m_certFolder;
				std::string m_sharedContentFolder;
                std::unordered_map<std::string, std::string> m_gameCerts;
                std::unordered_map<std::string, std::string> m_metadata;
                std::unordered_map<std::string, std::string> m_ports;
            };
        }
    }
}


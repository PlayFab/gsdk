#include "gsdkCommonPch.h"
#include "gsdkConfig.h"
#include "gsdk.h"
#include "gsdkUtils.h"
#include "fstream"

Microsoft::Azure::Gaming::ConfigurationBase::ConfigurationBase()
{
    // These are always set as environment variables, even with the new gsdk config json file
    m_titleId = cGSDKUtils::getEnvironmentVariable(Configuration::TITLE_ID_ENV_VAR);
    m_buildId = cGSDKUtils::getEnvironmentVariable(Configuration::BUILD_ID_ENV_VAR);
    m_region = cGSDKUtils::getEnvironmentVariable(Configuration::REGION_ENV_VAR);
}

const std::string &Microsoft::Azure::Gaming::ConfigurationBase::getTitleId()
{
    return m_titleId;
}

const std::string &Microsoft::Azure::Gaming::ConfigurationBase::getBuildId()
{
    return m_buildId;
}

const std::string &Microsoft::Azure::Gaming::ConfigurationBase::getRegion()
{
    return m_region;
}

bool Microsoft::Azure::Gaming::ConfigurationBase::shouldLog()
{
    return true;
}

bool Microsoft::Azure::Gaming::ConfigurationBase::shouldHeartbeat()
{
    return true;
}

Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::EnvironmentVariableConfiguration() : Microsoft::Azure::Gaming::ConfigurationBase::ConfigurationBase()
{
    m_heartbeatEndpoint = cGSDKUtils::getEnvironmentVariable(Configuration::HEARTBEAT_ENDPOINT_ENV_VAR);
    m_serverId = cGSDKUtils::getEnvironmentVariable(Configuration::SERVER_ID_ENV_VAR);
    m_logFolder = cGSDKUtils::getEnvironmentVariable(Configuration::LOG_FOLDER_ENV_VAR);
    m_sharedContentFolder = cGSDKUtils::getEnvironmentVariable(Configuration::SHARED_CONTENT_FOLDER_ENV_VAR);

    // Game cert support was added once we switched to a json config, so we don't have values for them
}

const std::string &Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::getHeartbeatEndpoint()
{
    return m_heartbeatEndpoint;
}

const std::string &Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::getServerId()
{
    return m_serverId;
}

const std::string &Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::getLogFolder()
{
    return m_logFolder;
}

const std::string &Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::getSharedContentFolder()
{
	return m_sharedContentFolder;
}


const std::string &Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::getCertificateFolder()
{
    return m_certFolder;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::getGameCertificates()
{
    return m_gameCerts;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::getBuildMetadata()
{
    return m_metadata;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::EnvironmentVariableConfiguration::getGamePorts()
{
    return m_ports;
}

Microsoft::Azure::Gaming::JsonFileConfiguration::JsonFileConfiguration(const std::string &file_name) : Microsoft::Azure::Gaming::ConfigurationBase::ConfigurationBase()
{
    std::ifstream is(file_name, std::ifstream::in);

    if (!is.fail())
    {
        Json::CharReaderBuilder jsonReaderFactory;
        Json::Value configFile;

        JSONCPP_STRING jsonParseErrors;
        bool parsedSuccessfully = Json::parseFromStream(jsonReaderFactory, is, &configFile, &jsonParseErrors);

        if (parsedSuccessfully)
        {
            m_heartbeatEndpoint = configFile["heartbeatEndpoint"].asString();
            m_serverId = configFile["sessionHostId"].asString();
            m_logFolder = configFile["logFolder"].asString();
			m_sharedContentFolder = configFile["sharedContentFolder"].asString();

            if (configFile.isMember("certificateFolder"))
            {
                m_certFolder = configFile["certificateFolder"].asString();
            }
            else
            {
                m_certFolder = std::string();
            }

            if (configFile.isMember("gameCertificates"))
            {
                Json::Value gameCerts = configFile["gameCertificates"];
                for (Json::ValueIterator i = gameCerts.begin(); i != gameCerts.end(); ++i)
                {
                    m_gameCerts[i.key().asCString()] = (*i).asCString();
                }
            }

            if (configFile.isMember("buildMetadata"))
            {
                Json::Value metadata = configFile["buildMetadata"];
                for (Json::ValueIterator i = metadata.begin(); i != metadata.end(); ++i)
                {
                    m_metadata[i.key().asCString()] = (*i).asCString();
                }
            }

            if (configFile.isMember("gamePorts"))
            {
                Json::Value ports = configFile["gamePorts"];
                for (Json::ValueIterator i = ports.begin(); i != ports.end(); ++i)
                {
                    m_ports[i.key().asCString()] = (*i).asCString();
                }
            }
        }
        else
        {
            // No logging setup at this point, so throw an exception
            throw GSDKInitializationException("Failed to parse configuration: " + jsonParseErrors);
        }
    }
    else
    {
        throw GSDKInitializationException("Failed to open configuration file: " + file_name);
    }
}

const std::string &Microsoft::Azure::Gaming::JsonFileConfiguration::getHeartbeatEndpoint()
{
    return m_heartbeatEndpoint;
}

const std::string &Microsoft::Azure::Gaming::JsonFileConfiguration::getServerId()
{
    return m_serverId;
}

const std::string &Microsoft::Azure::Gaming::JsonFileConfiguration::getLogFolder()
{
    return m_logFolder;
}

const std::string &Microsoft::Azure::Gaming::JsonFileConfiguration::getSharedContentFolder()
{
	return m_sharedContentFolder;
}

const std::string &Microsoft::Azure::Gaming::JsonFileConfiguration::getCertificateFolder()
{
    return m_certFolder;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::JsonFileConfiguration::getGameCertificates()
{
    return m_gameCerts;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::JsonFileConfiguration::getBuildMetadata()
{
    return m_metadata;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::JsonFileConfiguration::getGamePorts()
{
    return m_ports;
}
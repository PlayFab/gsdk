#include "stdafx.h"
#include "TestConfig.h"

Microsoft::Azure::Gaming::TestConfig::TestConfig(   const std::string & heartbeatEndpoint,
                                                    const std::string & serverId,
                                                    const std::string & logFolder,
                                                    const std::string & sharedContentFolder,
                                                    const std::string & certFolder,
                                                    const std::unordered_map<std::string, std::string> & gameCerts,
                                                    const std::string & titleId,
                                                    const std::string & buildId,
                                                    const std::string & region,
                                                    const std::unordered_map<std::string, std::string> & metadata,
                                                    const std::unordered_map<std::string, std::string> & ports)
{
    m_heartbeatEndpoint = heartbeatEndpoint;
    m_serverId = serverId;
    m_logFolder = logFolder;
    m_sharedContentFolder = sharedContentFolder;
    m_certFolder = certFolder;
    m_gameCerts = gameCerts;
    m_titleId = titleId;
    m_buildId = buildId;
    m_region = region;
    m_metadata = metadata;
    m_ports = ports;
}

const std::string &Microsoft::Azure::Gaming::TestConfig::getHeartbeatEndpoint()
{
    return m_heartbeatEndpoint;
}

const std::string &Microsoft::Azure::Gaming::TestConfig::getServerId()
{
    return m_serverId;
}

const std::string &Microsoft::Azure::Gaming::TestConfig::getLogFolder()
{
    return m_logFolder;
}

const std::string &Microsoft::Azure::Gaming::TestConfig::getSharedContentFolder()
{
    return m_sharedContentFolder;
}

const std::string &Microsoft::Azure::Gaming::TestConfig::getCertificateFolder()
{
    return m_certFolder;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::TestConfig::getGameCertificates()
{
    return m_gameCerts;
}

const std::string &Microsoft::Azure::Gaming::TestConfig::getTitleId()
{
    return m_titleId;
}

const std::string &Microsoft::Azure::Gaming::TestConfig::getBuildId()
{
    return m_buildId;
}

const std::string &Microsoft::Azure::Gaming::TestConfig::getRegion()
{
    return m_region;
}

bool Microsoft::Azure::Gaming::TestConfig::shouldLog()
{
    return false;
}

bool Microsoft::Azure::Gaming::TestConfig::shouldHeartbeat()
{
    return false;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::TestConfig::getBuildMetadata()
{
    return m_metadata;
}

const std::unordered_map<std::string, std::string> &Microsoft::Azure::Gaming::TestConfig::getGamePorts()
{
    return m_ports;
}
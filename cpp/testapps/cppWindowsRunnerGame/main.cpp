
// Copyright (C) Microsoft Corporation. All rights reserved.

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <string.h>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <future>
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#include <cryptuiapi.h>

#include "gsdk.h"

#define PORT 3600

static int requestCount = 0;
static time_t nextMaintenance;
static std::string assetFileTextPath = "testassetfile.txt";
static std::string assetFileText;
static std::string installedCertThumbprint;
static std::string cmdArgs;
static bool isActivated = false;
static bool isShutdown = false;
static bool delayShutdown = false;
static bool isMaintenancedScheduled = false;
static std::vector<Microsoft::Azure::Gaming::ConnectedPlayer> players;

static std::future<void> processRequestsThread;
static struct sockaddr_in address;
static int server_fd;


void inShutdown()
{
    printf("GSDK is shutting me down!!!\n");
    fflush(stdout);
    isShutdown = true;

    if (!delayShutdown)
    {
        std::exit(0);
    }
}

bool isHealthy()
{
    return requestCount % 2 == 0 ? true : false;
}

void maintenanceScheduled(tm t)
{
    nextMaintenance = mktime(&t);
    isMaintenancedScheduled = true;
}

void maintenanceV2Scheduled(Microsoft::Azure::Gaming::MaintenanceSchedule schedule)
{
    printf("GSDK maintenanceV2 scheduled with %s, %s, %s\n", schedule.m_events[0].m_eventType.c_str(), schedule.m_events[0].m_eventStatus.c_str(), schedule.m_events[0].m_eventSource.c_str());
    fflush(stdout);
}

std::string escape(std::string const &s)
{
    std::size_t n = s.length();
    std::string escaped;
    escaped.reserve(n * 2);

    for (std::size_t i = 0; i < n; ++i) {
        if (s[i] == '\\' || s[i] == '\"')
            escaped += '\\';
        escaped += s[i];
    }
    return escaped;
}

std::string getExecutablePath()
{
    HMODULE hModule = GetModuleHandleW(NULL);
    WCHAR pathBuffer[MAX_PATH];
    GetModuleFileNameW(hModule, pathBuffer, MAX_PATH);
    char ch[260];
    char DefChar = ' ';
    WideCharToMultiByte(CP_ACP, 0, pathBuffer, -1, ch, 260, &DefChar, NULL);
    return std::string(ch);
}

std::string getExecutableDirectory()
{
    std::string path = getExecutablePath();
    int lastIndex = path.find_last_of('\\');
    return path.substr(0, lastIndex);
}

void processRequests()
{
    while (true)
    {
        int new_socket;
        int valread;
        int addrlen = sizeof(address);
        char buffer[1024] = { 0 };

        if ((new_socket = accept(server_fd, (sockaddr*)&address, &addrlen)) < 0)
        {
            perror("accept");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        valread = recv(new_socket, buffer, sizeof(buffer), 0);
        printf("HTTP:Received %.*s\n", valread, buffer);
        fflush(stdout);

        // For each request, "add" a connected player
        players.push_back(Microsoft::Azure::Gaming::ConnectedPlayer("gamer" + std::to_string(requestCount)));
        requestCount++;
        Microsoft::Azure::Gaming::GSDK::updateConnectedPlayers(players);

        std::unordered_map<std::string, std::string> config = Microsoft::Azure::Gaming::GSDK::getConfigSettings();

        // First, check if we need to delay shutdown for testing
        auto it = config.find(Microsoft::Azure::Gaming::GSDK::SESSION_COOKIE_KEY);

        if (it != config.end() && strcmp(it->second.c_str(), "delayshutdown") == 0)
        {
            delayShutdown = true;
        }

        if (isActivated)
        {
            std::string playersJoinedAsString = "";
            for (auto player : Microsoft::Azure::Gaming::GSDK::getInitialPlayers())
            {
                playersJoinedAsString += (playersJoinedAsString.empty() ? "" : ",") + player;
            }

            config["initialPlayers"] = playersJoinedAsString;
        }

        config["isActivated"] = isActivated ? "true" : "false";
        config["isShutdown"] = isShutdown ? "true" : "false";
        config["assetFileText"] = assetFileText;
        config["installedCertThumbprint"] = installedCertThumbprint;
        config["cmdArgs"] = cmdArgs;

        if (isMaintenancedScheduled)
        {
            char buffer[10];
            tm tm;
            localtime_s(&tm, &nextMaintenance);
            asctime_s(buffer, sizeof(buffer), &tm);
            std::string timeStr(buffer);
            timeStr.erase(std::remove(timeStr.begin(), timeStr.end(), '\n'), timeStr.end());
            config["nextMaintenance"] = timeStr;
        }

        // Format the response
        bool first = true;
        std::string content;
        content.reserve(1024);
        content += "{\r\n";
        for (auto configVal : config)
        {
            if (!first) {
                content += ",\r\n";
            }

            content += "\"";
            content += configVal.first;
            content += "\"";
            content += ": ";
            content += "\"";
            content += escape(configVal.second);
            content += "\"";

            first = false;
        }
        content += "\r\n}";

        char dateBuf[1000];
        time_t now = time(0);
        struct tm tm;
        gmtime_s(&tm, &now);
        strftime(dateBuf, sizeof dateBuf, "%a, %d %b %Y %H:%M:%S %Z", &tm);

        std::string reply;
        reply.reserve(2048);
        reply += "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: ";
        reply += std::to_string(content.size());
        reply += "\r\nDate: ";
        reply += dateBuf;
        reply += "\r\n\r\n";
        reply += content;

        send(new_socket, reply.c_str(), reply.size(), 0);

        // If we were shutdown, that was the last reply we would send
        if (isShutdown)
        {
            std::exit(0);
        }
    }
}

std::string hex_encode(unsigned char* data, size_t len)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; i++)
    {
        // Cast data[i] to an int so that it uses the int-based overload of the << operator.
        // If it uses the char-based overload then it's going to treat data[i] as a printable character.
        // See: [https://arstechnica.com/civis/viewtopic.php?t=796634](https://arstechnica.com/civis/viewtopic.php?t=796634)
        ss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return ss.str();
}

bool DoesCertificateExist(std::string expectedThumbprint)
{
    bool exist = false;
    HCERTSTORE hCertStore = CertOpenStore(
        CERT_STORE_PROV_SYSTEM,
        0,
        NULL,
        CERT_SYSTEM_STORE_LOCAL_MACHINE,
        L"MY"
    );

    if (!hCertStore)
    {
        return exist;
    }

    DWORD dwPropId = 0;
    char pszNameString[256];
    char pszStoreName[256];
    void*            pvData;
    DWORD            cbData;
    PCCERT_CONTEXT   pCertContext = NULL;
    while (pCertContext = CertEnumCertificatesInStore(
        hCertStore,
        pCertContext))
    {
        while (dwPropId = CertEnumCertificateContextProperties(
            pCertContext,
            dwPropId))
        {
            if (dwPropId != CERT_SHA1_HASH_PROP_ID)
            {
                continue;
            }

            if (!CertGetCertificateContextProperty(
                pCertContext,
                dwPropId,
                NULL,
                &cbData))
            {
                printf("Call #1 to GetCertContextProperty failed.");
                fflush(stdout);
                continue;
            }

            pvData = (void*)malloc(cbData);
            if (!CertGetCertificateContextProperty(
                pCertContext,
                dwPropId,
                pvData,
                &cbData))
            {
                free(pvData);
                continue;
            }

            std::string value = hex_encode((BYTE*)pvData, cbData);
            if (_strcmpi(value.c_str(), expectedThumbprint.c_str()) == 0)
            {
                exist = true;
            }

            free(pvData);
        }
    }

    if (pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    CertCloseStore(hCertStore, 0);
    return exist;
}

void getTestCert()
{
    std::unordered_map<std::string, std::string> config = Microsoft::Azure::Gaming::GSDK::getConfigSettings();
    auto it = config.find("winRunnerTestCert");

    if (it != config.end())
    {
        std::string expectedThumbprint = config["winRunnerTestCert"];
        if (DoesCertificateExist(expectedThumbprint))
        {
            installedCertThumbprint = expectedThumbprint;
        }
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // Index starts from 1 to exclude executable path
        for (int i = 1; i < argc - 1; i++)
        {
            cmdArgs = cmdArgs + argv[i] + " ";
        }

        if (argc > 1)
        {
            cmdArgs = cmdArgs + argv[argc - 1];
        }

        Microsoft::Azure::Gaming::GSDK::start();
        Microsoft::Azure::Gaming::GSDK::registerShutdownCallback(&inShutdown);
        Microsoft::Azure::Gaming::GSDK::registerHealthCallback(&isHealthy);
        //Microsoft::Azure::Gaming::GSDK::registerMaintenanceCallback(&maintenanceScheduled);
        Microsoft::Azure::Gaming::GSDK::registerMaintenanceV2Callback(&maintenanceV2Scheduled);

        // Grab asset files
        std::ifstream assetFileTextStream(getExecutableDirectory() + "\\" + assetFileTextPath);
        if (assetFileTextStream.good()) {
            getline(assetFileTextStream, assetFileText);
            assetFileTextStream.close();
        }
        
        // Grab cert
        getTestCert();

        char opt = 1;

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        {
            perror("setsockopt");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            perror("bind failed");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, SOMAXCONN) < 0)
        {
            perror("listen");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        processRequestsThread = std::async(std::launch::async, &processRequests);

        if (Microsoft::Azure::Gaming::GSDK::readyForPlayers())
        {
            isActivated = true;
        }

        processRequestsThread.wait();
    }
    catch (const std::exception &ex)
    {
        printf("Exception encountered: %s", ex.what());
        fflush(stdout);
        Microsoft::Azure::Gaming::GSDK::logMessage(ex.what());
    }

    return 0;
}
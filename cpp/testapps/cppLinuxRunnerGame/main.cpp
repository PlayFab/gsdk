
// Copyright (C) Microsoft Corporation. All rights reserved.

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <fstream>
#include <algorithm>
#include <future>

#include "gsdk.h"

#define PORT 3600

static int requestCount = 0;
static time_t nextMaintenance;
static std::string assetFileTextPath = "/data/Assets/testassetfile.txt";
static std::string assetFileTarPath = "/data/AssetsTar/testassetfile.txt";
static std::string assetFileTarGzPath = "/data/AssetsTarGz/testassetfile.txt";
static std::string assetFileText;
static std::string assetFileTar;
static std::string assetFileTarGz;
static std::string testCertificate;
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
    nextMaintenance = timegm(&t);
    isMaintenancedScheduled = true;
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

void processRequests()
{
    while (true)
    {
        int new_socket;
        ssize_t valread;
        int addrlen = sizeof(address);
        char buffer[1024] = { 0 };

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
            (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        valread = read(new_socket, buffer, 1024);
        printf("HTTP:Received %.*s\n", valread, buffer);

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

        config["isActivated"] = isActivated? "true" : "false";
        config["isShutdown"] = isShutdown? "true" : "false";
        config["assetFileText"] = assetFileText;
        config["assetFileTar"] = assetFileTar;
        config["assetFileTarGz"] = assetFileTarGz;
        config["testCertificate"] = testCertificate;

        if (isMaintenancedScheduled)
        {
            std::string timeStr(asctime(localtime(&nextMaintenance)));
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
        struct tm tm = *gmtime(&now);
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

void getTestCert()
{
    std::unordered_map<std::string, std::string> config = Microsoft::Azure::Gaming::GSDK::getConfigSettings();
    auto it = config.find("certificateFolder");

    if (it != config.end())
    {
        std::string testCertificatePath = config["certificateFolder"] + "/LinuxRunnerTestCert.pem";

        FILE *fp = fopen(testCertificatePath.c_str(), "r");
        if (fp) {
            X509 *cert = PEM_read_X509(fp, NULL, NULL, NULL);
            if (cert != NULL) {
                // get thumbprint
                const int sha1Len = 20;
                unsigned char sha1Buf[sha1Len];
                const EVP_MD *digest = EVP_sha1();
                unsigned bufLen;
                int sha1Res = X509_digest(cert, digest, sha1Buf, &bufLen);
                if (sha1Res != 0)
                {
                    testCertificate = hex_encode(sha1Buf, bufLen);
                }

                X509_free(cert);
            }
            fclose(fp);
        }
    }
}

int main()
{
    try
    {
        Microsoft::Azure::Gaming::GSDK::start();
        Microsoft::Azure::Gaming::GSDK::registerShutdownCallback(&inShutdown);
        Microsoft::Azure::Gaming::GSDK::registerHealthCallback(&isHealthy);
        Microsoft::Azure::Gaming::GSDK::registerMaintenanceCallback(&maintenanceScheduled);

        // Grab asset files
        std::ifstream assetFileTextStream(assetFileTextPath);
        if (assetFileTextStream.good()) {
            getline(assetFileTextStream, assetFileText);
            assetFileTextStream.close();
        }

        std::ifstream assetFileTarStream(assetFileTarPath);
        if (assetFileTarStream.good()) {
            getline(assetFileTarStream, assetFileTar);
            assetFileTarStream.close();
        }

        std::ifstream assetFileTarGzStream(assetFileTarGzPath);
        if (assetFileTarGzStream.good()) {
            getline(assetFileTarGzStream, assetFileTarGz);
            assetFileTarGzStream.close();
        }

        // Grab cert
        getTestCert();

        int opt = 1;

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
            &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&address,
            sizeof(address))<0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, SOMAXCONN) < 0)
        {
            perror("listen");
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
        Microsoft::Azure::Gaming::GSDK::logMessage(Microsoft::Azure::Gaming::GSDKLogLevel::Error, ex.what());
    }

    return 0;
}
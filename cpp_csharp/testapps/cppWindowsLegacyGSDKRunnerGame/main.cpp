#include <cstdio>
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <string.h>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <future>
#include <winsock2.h>
#include <stddef.h>
#include <time.h>

#include "GSDK.h"

#pragma comment(lib, "Ws2_32.lib")

#define PORT 3600

static bool isActivated = false;
static bool isShutdown = false;
static bool delayShutdown = false;
static int requestCount = 0;

static std::future<void> processRequestsThread;
static struct sockaddr_in address;
static int server_fd;

void inShutdown(LPVOID state)
{
    printf("GSDK is shutting me down!!!\n");
    isShutdown = true;

    if (!delayShutdown)
    {
        std::exit(0);
    }
}

void inQuarantine(LPVOID state)
{
    printf("GSDK is quarantining me!!!\n");
}

void getTestCert()
{
    // Does nothing now
    return;
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

std::string createResponseContent()
{
    std::unordered_map<std::string, std::string> config = std::unordered_map<std::string, std::string>();
    config["isActivated"] = isActivated ? "true" : "false";
    config["isShutdown"] = isShutdown ? "true" : "false";
    config["sessionCookie"] = "NA";
    config["logFolder"] = "NA";
    config["certificateFolder"] = "NA";
    config["assetFileText"] = "NA";
    config["winRunnerTestCert"] = "NA";
    config["installedCertThumbprint"] = "NA";
    config["cmdArgs"] = "NA";
    config["nextMaintenance"] = "NA";


    std::string content;
    bool first = true;
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

    return content;
}

void processRequests()
{
    while (true)
    {
        int new_socket;
        int valread;
        int addrlen = sizeof(address);
        char buffer[1024] = { 0 };

        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int activity = select(server_fd + 1, &readfds, NULL, NULL, &tv);

        if (activity <= 0)
        {
            if (isShutdown)
            {
                break;
            }
            else
            {
                continue;
            }
        }

        if ((new_socket = (int)accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        valread = recv(new_socket, buffer, 1024, 0);
        printf("HTTP:Received %.*s\n", valread, buffer);
        requestCount++;

        // Format the response
        bool first = true;
        std::string content = createResponseContent();

        char dateBuf[1000];
        time_t now = time(0);
        struct tm buf;
        gmtime_s(&buf, &now);
        asctime_s(dateBuf, sizeof dateBuf, &buf);

        std::string reply;
        reply.reserve(2048);
        reply += "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: ";
        reply += std::to_string(content.size());
        reply += "\r\nDate: ";
        reply += dateBuf;
        reply += "\r\n\r\n";
        reply += content;

        send(new_socket, reply.c_str(), (int)(reply.size()), 0);

        // If we were shutdown, that was the last reply we would send
        if (isShutdown)
        {
            break;
        }
    }
}

int main()
{
    try
    {
        HRESULT result = GSDKStartup();
        if (!SUCCEEDED(result))
        {
            perror("GSDKStartup failed");
            exit(EXIT_FAILURE);
        }

        result = GSDKRegisterShutdownCallback(inShutdown, 0);
        if (!SUCCEEDED(result))
        {
            perror("GSDKRegisterShutdownCallback failed");
            exit(EXIT_FAILURE);
        }

        result = GSDKRegisterQuarantinedCallback(inQuarantine, 0);
        if (!SUCCEEDED(result))
        {
            perror("GSDKRegisterQuarantinedCallback failed");
            exit(EXIT_FAILURE);
        }

        // Grab cert
        getTestCert();

        int opt = 1;

        // Creating socket file descriptor
        if ((server_fd = (int)socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
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

        result = GSDKWaitForSessionAssignment();
        if (SUCCEEDED(result))
        {
            isActivated = true;
        }

        processRequestsThread.wait();
    }
    catch (const std::exception &ex)
    {
        printf("Exception encountered: %s", ex.what());
    }

    return 0;
}
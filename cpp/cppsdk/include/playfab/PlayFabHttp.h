#pragma once

#include <gsdkCommonPch.h>

#include <playfab/PlayFabError.h>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>

namespace PlayFab
{
    struct CallRequestContainer;
    typedef void(*RequestCompleteCallback)(CallRequestContainer& reqContainer);
    typedef std::shared_ptr<void> SharedVoidPointer;

    /// <summary>
    /// Internal PlayFabHttp container for each api call
    /// </summary>
    struct CallRequestContainer
    {
        // I own these objects, I must always destroy them
        CURL* curlHandle;
        curl_slist* curlHttpHeaders;
        // I never own this, I can never destroy it
        void* customData;

        bool finished;
        std::string authKey;
        std::string authValue;
        std::string responseString;
        Json::Value responseJson = Json::Value::null;
        PlayFabError errorWrapper;
        RequestCompleteCallback internalCallback;
        SharedVoidPointer successCallback;
        ErrorCallback errorCallback;

        CallRequestContainer();
        ~CallRequestContainer();
    };

    /// <summary>
    /// Provides an interface and a static instance for https implementations
    /// </summary>
    class IPlayFabHttp
    {
    public:
        static IPlayFabHttp& Get();

        virtual ~IPlayFabHttp();

        virtual void AddRequest(const std::string& urlPath, const std::string& authKey, const std::string& authValue, const Json::Value& requestBody, RequestCompleteCallback internalCallback, SharedVoidPointer successCallback, ErrorCallback errorCallback, void* customData) = 0;
        virtual size_t Update() = 0;
    protected:
        static std::unique_ptr<IPlayFabHttp> httpInstance;
    };

    /// <summary>
    /// PlayFabHttp is the default https implementation for Win/C++, using cpprestsdk
    /// </summary>
    class PlayFabHttp : public IPlayFabHttp
    {
    public:
        static void MakeInstance();
        ~PlayFabHttp() override;

        void AddRequest(const std::string& urlPath, const std::string& authKey, const std::string& authValue, const Json::Value& requestBody, RequestCompleteCallback internalCallback, SharedVoidPointer successCallback, ErrorCallback errorCallback, void* customData) override;
        size_t Update() override;
    private:
        PlayFabHttp(); // Private constructor, to enforce singleton instance
        PlayFabHttp(const PlayFabHttp& other); // Private copy-constructor, to enforce singleton instance

        static size_t CurlReceiveData(char* buffer, size_t blockSize, size_t blockCount, void* userData);
        static void ExecuteRequest(CallRequestContainer& reqContainer);
        void WorkerThread();
        static void HandleCallback(CallRequestContainer& reqContainer);
        static void HandleResults(CallRequestContainer& reqContainer);

        std::thread pfHttpWorkerThread;
        std::mutex httpRequestMutex;
        bool threadRunning;
        std::vector<CallRequestContainer*> pendingRequests;
        std::vector<CallRequestContainer*> pendingResults;
    };
}

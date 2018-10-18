#include <gsdkCommonPch.h>

#include <playfab/PlayFabHttp.h>
#include <playfab/PlayFabSettings.h>
#include <exception>

namespace PlayFab
{
    CallRequestContainer::CallRequestContainer() :
        curlHandle(nullptr),
        curlHttpHeaders(nullptr),
        customData(nullptr),
        finished(false),
        responseString(""),
        responseJson(Json::Value::null),
        errorWrapper(),
        internalCallback(nullptr),
        successCallback(nullptr),
        errorCallback(nullptr)
    {
    }

    CallRequestContainer::~CallRequestContainer()
    {
        curl_easy_reset(curlHandle);
        curlHttpHeaders = nullptr;
    }

    std::unique_ptr<IPlayFabHttp> IPlayFabHttp::httpInstance = nullptr;
    IPlayFabHttp::~IPlayFabHttp() = default;
    IPlayFabHttp& IPlayFabHttp::Get()
    {
        // In the future we could make it easier to override this instance with a sub-type, for now it defaults to the only one we have
        if (httpInstance == nullptr)
            PlayFabHttp::MakeInstance();
        return *httpInstance.get();
    }

    PlayFabHttp::PlayFabHttp()
    {
        threadRunning = true;
        pfHttpWorkerThread = std::thread(&PlayFabHttp::WorkerThread, this);
    };

    PlayFabHttp::~PlayFabHttp()
    {
        threadRunning = false;
        pfHttpWorkerThread.join();
        for (size_t i = 0; i < pendingRequests.size(); ++i)
            delete pendingRequests[i];
        pendingRequests.clear();
        for (size_t i = 0; i < pendingResults.size(); ++i)
            delete pendingResults[i];
        pendingResults.clear();
    }

    void PlayFabHttp::MakeInstance()
    {
        class _PlayFabHttp : public PlayFabHttp {}; // Hack to bypass private constructor on PlayFabHttp

        if (httpInstance == nullptr)
        {
            httpInstance = std::make_unique<_PlayFabHttp>();
        }
    }

    void PlayFabHttp::WorkerThread()
    {
        size_t queueSize;

        while (this->threadRunning)
        {
            CallRequestContainer* reqContainer = nullptr;

            { // LOCK httpRequestMutex
                std::unique_lock<std::mutex> lock(this->httpRequestMutex);

                queueSize = this->pendingRequests.size();
                if (queueSize != 0)
                {
                    reqContainer = this->pendingRequests[this->pendingRequests.size() - 1];
                    this->pendingRequests.pop_back();
                }
            } // UNLOCK httpRequestMutex

            if (queueSize == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            if (reqContainer != nullptr)
                ExecuteRequest(*reqContainer);
        }
    }

    void PlayFabHttp::HandleCallback(CallRequestContainer& reqContainer)
    {
        reqContainer.finished = true;
        if (PlayFabSettings::threadedCallbacks)
            HandleResults(reqContainer);

        PlayFabHttp& instance = reinterpret_cast<PlayFabHttp&>(Get());
        if (!PlayFabSettings::threadedCallbacks)
        {
            { // LOCK httpRequestMutex
                std::unique_lock<std::mutex> lock(instance.httpRequestMutex);
                instance.pendingResults.push_back(&reqContainer);
            } // UNLOCK httpRequestMutex
        }
    }

    size_t PlayFabHttp::CurlReceiveData(char* buffer, size_t blockSize, size_t blockCount, void* userData)
    {
        CallRequestContainer* reqContainer = reinterpret_cast<CallRequestContainer*>(userData);
        reqContainer->responseString.assign(buffer, blockSize * blockCount);

        Json::CharReaderBuilder jsonReaderFactory;
        Json::CharReader* jsonReader(jsonReaderFactory.newCharReader());
        JSONCPP_STRING jsonParseErrors;
        const bool parsedSuccessfully = jsonReader->parse(reqContainer->responseString.c_str(), reqContainer->responseString.c_str() + reqContainer->responseString.length(), &reqContainer->responseJson, &jsonParseErrors);

        if (parsedSuccessfully)
        {
            reqContainer->errorWrapper.HttpCode = reqContainer->responseJson.get("code", Json::Value::null).asInt();
            reqContainer->errorWrapper.HttpStatus = reqContainer->responseJson.get("status", Json::Value::null).asString();
            reqContainer->errorWrapper.Data = reqContainer->responseJson.get("data", Json::Value::null);
            reqContainer->errorWrapper.ErrorName = reqContainer->responseJson.get("error", Json::Value::null).asString();
            reqContainer->errorWrapper.ErrorMessage = reqContainer->responseJson.get("errorMessage", Json::Value::null).asString();
            reqContainer->errorWrapper.ErrorDetails = reqContainer->responseJson.get("errorDetails", Json::Value::null);
        }
        else
        {
            reqContainer->errorWrapper.HttpCode = 408;
            reqContainer->errorWrapper.HttpStatus = reqContainer->responseString;
            reqContainer->errorWrapper.ErrorCode = PlayFabErrorConnectionTimeout;
            reqContainer->errorWrapper.ErrorName = "Failed to parse PlayFab response";
            reqContainer->errorWrapper.ErrorMessage = jsonParseErrors;
        }

        HandleCallback(*reqContainer);
        return (blockSize * blockCount);
    }

    void PlayFabHttp::AddRequest(const std::string& urlPath, const std::string& authKey, const std::string& authValue, const Json::Value& requestBody, RequestCompleteCallback internalCallback, SharedVoidPointer successCallback, ErrorCallback errorCallback, void* customData)
    {
        CallRequestContainer* reqContainer = new CallRequestContainer();
        reqContainer->errorWrapper.UrlPath = urlPath;
        reqContainer->authKey = authKey;
        reqContainer->authValue = authValue;
        reqContainer->errorWrapper.Request = requestBody;
        reqContainer->internalCallback = internalCallback;
        reqContainer->successCallback = successCallback;
        reqContainer->errorCallback = errorCallback;
        reqContainer->customData = customData;

        { // LOCK httpRequestMutex
            std::unique_lock<std::mutex> lock(httpRequestMutex);
            pendingRequests.push_back(reqContainer);
        } // UNLOCK httpRequestMutex
    }

    void PlayFabHttp::ExecuteRequest(CallRequestContainer& reqContainer)
    {
        // Set up curl handle
        reqContainer.curlHandle = curl_easy_init();
        curl_easy_reset(reqContainer.curlHandle);
        curl_easy_setopt(reqContainer.curlHandle, CURLOPT_URL, PlayFabSettings::GetUrl(reqContainer.errorWrapper.UrlPath).c_str());

        // Set up headers
        reqContainer.curlHttpHeaders = nullptr;
        reqContainer.curlHttpHeaders = curl_slist_append(reqContainer.curlHttpHeaders, "Accept: application/json");
        reqContainer.curlHttpHeaders = curl_slist_append(reqContainer.curlHttpHeaders, "Content-Type: application/json; charset=utf-8");
        reqContainer.curlHttpHeaders = curl_slist_append(reqContainer.curlHttpHeaders, ("X-PlayFabSDK: " + PlayFabSettings::versionString).c_str());
        reqContainer.curlHttpHeaders = curl_slist_append(reqContainer.curlHttpHeaders, "X-ReportErrorAsSuccess: true");
        if (reqContainer.authKey.length() != 0 && reqContainer.authValue.length() != 0)
            reqContainer.curlHttpHeaders = curl_slist_append(reqContainer.curlHttpHeaders, (reqContainer.authKey + ": " + reqContainer.authValue).c_str());
        curl_easy_setopt(reqContainer.curlHandle, CURLOPT_HTTPHEADER, reqContainer.curlHttpHeaders);

        // Set up post & payload
        std::string payload = reqContainer.errorWrapper.Request.toStyledString();
        curl_easy_setopt(reqContainer.curlHandle, CURLOPT_POST, nullptr);
        curl_easy_setopt(reqContainer.curlHandle, CURLOPT_POSTFIELDS, payload.c_str());

        // Process result
        // TODO: CURLOPT_ERRORBUFFER ?
        curl_easy_setopt(reqContainer.curlHandle, CURLOPT_TIMEOUT_MS, 10000L);
        curl_easy_setopt(reqContainer.curlHandle, CURLOPT_WRITEDATA, &reqContainer);
        curl_easy_setopt(reqContainer.curlHandle, CURLOPT_WRITEFUNCTION, CurlReceiveData);

        // Send
        curl_easy_setopt(reqContainer.curlHandle, CURLOPT_SSL_VERIFYPEER, false); // TODO: Replace this with a ca-bundle ref???
        const auto res = curl_easy_perform(reqContainer.curlHandle);
        if (res != CURLE_OK)
        {
            reqContainer.errorWrapper.HttpCode = 408;
            reqContainer.errorWrapper.HttpStatus = "Failed to contact server";
            reqContainer.errorWrapper.ErrorCode = PlayFabErrorConnectionTimeout;
            reqContainer.errorWrapper.ErrorName = "Failed to contact server";
            reqContainer.errorWrapper.ErrorMessage = "Failed to contact server, curl error: " + std::to_string(res);
            HandleCallback(reqContainer);
        }
    }

    void PlayFabHttp::HandleResults(CallRequestContainer& reqContainer)
    {
        // The success case must be handled by a function which is aware of the ResultType
        if (reqContainer.errorWrapper.HttpCode == 200)
        {
            reqContainer.internalCallback(reqContainer); // Unpacks the result as ResultType and invokes successCallback according to that type
        }
        else // Process the error case
        {
            if (PlayFabSettings::globalErrorHandler != nullptr)
                PlayFabSettings::globalErrorHandler(reqContainer.errorWrapper, reqContainer.customData);
            if (reqContainer.errorCallback != nullptr)
                reqContainer.errorCallback(reqContainer.errorWrapper, reqContainer.customData);
        }
    }

    size_t PlayFabHttp::Update()
    {
        //if (PlayFabSettings::threadedCallbacks)
        //    throw std::exception("You should not call Update() when PlayFabSettings::threadedCallbacks == true");

        CallRequestContainer* reqContainer;
        size_t resultCount;

        { // LOCK httpRequestMutex
            std::unique_lock<std::mutex> lock(httpRequestMutex);
            resultCount = pendingResults.size();
            if (resultCount == 0)
                return resultCount;

            reqContainer = pendingResults[pendingResults.size() - 1];
            pendingResults.pop_back();
        } // UNLOCK httpRequestMutex

        HandleResults(*reqContainer);
        delete reqContainer;
        return resultCount;
    }
}

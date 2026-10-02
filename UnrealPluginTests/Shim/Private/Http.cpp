// Copyright (C) Microsoft Corporation. All rights reserved.

#include <chrono>
#include <vector>

#include "ShimInternal.h"

namespace
{
	std::mutex& HandlerMutex()
	{
		static std::mutex* Mutex = new std::mutex();
		return *Mutex;
	}

	UEShim::FFakeHttpHandler& Handler()
	{
		static UEShim::FFakeHttpHandler* Instance = new UEShim::FFakeHttpHandler();
		return *Instance;
	}

	class FShimHttpResponse : public IHttpResponse
	{
	public:
		FShimHttpResponse(const FString& InUrl, int32 InCode, const FString& InBody) : Url(InUrl), Code(InCode), Body(InBody) {}

		FString GetURL() const override { return Url; }
		FString GetContentAsString() const override { return Body; }
		uint64 GetContentLength() const override { return UEShim::ToUtf8(Body.GetStdString()).size(); }
		int32 GetResponseCode() const override { return Code; }

	private:
		FString Url;
		int32 Code;
		FString Body;
	};

	class FShimHttpRequest;

	struct FPendingCompletion
	{
		std::shared_ptr<FShimHttpRequest> Request;
		std::chrono::steady_clock::time_point ReadyTime;
	};

	std::mutex& PendingMutex()
	{
		static std::mutex* Mutex = new std::mutex();
		return *Mutex;
	}

	std::vector<FPendingCompletion>& PendingCompletions()
	{
		static std::vector<FPendingCompletion>* Pending = new std::vector<FPendingCompletion>();
		return *Pending;
	}

	class FShimHttpRequest : public IHttpRequest, public std::enable_shared_from_this<FShimHttpRequest>
	{
	public:
		FString GetURL() const override { return Url; }
		FString GetContentAsString() const override { return Content; }
		uint64 GetContentLength() const override { return UEShim::ToUtf8(Content.GetStdString()).size(); }
		FString GetVerb() const override { return Verb; }
		void SetURL(const FString& InUrl) override { Url = InUrl; }
		void SetVerb(const FString& InVerb) override { Verb = InVerb; }
		void SetHeader(const FString& HeaderName, const FString& HeaderValue) override { Headers.Add(HeaderName, HeaderValue); }
		void SetContentAsString(const FString& InContent) override { Content = InContent; }
		FHttpRequestCompleteDelegate& OnProcessRequestComplete() override { return CompleteDelegate; }

		bool ProcessRequest() override
		{
			UEShim::FFakeHttpHandler CurrentHandler;
			{
				std::lock_guard<std::mutex> Lock(HandlerMutex());
				CurrentHandler = Handler();
			}

			FHttpResponsePtr NewResponse;
			if (CurrentHandler)
			{
				const UEShim::FFakeHttpResponse Result = CurrentHandler(Verb, Url, Content);
				NewResponse = MakeShared<FShimHttpResponse>(Url, Result.Code, Result.Body);
			}

			{
				std::lock_guard<std::mutex> Lock(Mutex);
				Response = NewResponse;
				bConnectedSuccessfully = NewResponse.IsValid();
			}

			// Completion delegates run on the game thread a little later, like Unreal's HTTP manager tick; this also
			// leaves time for callers that bind OnProcessRequestComplete() after calling ProcessRequest().
			std::lock_guard<std::mutex> Lock(PendingMutex());
			PendingCompletions().push_back({ shared_from_this(), std::chrono::steady_clock::now() + std::chrono::milliseconds(10) });
			return true;
		}

		void CancelRequest() override
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			bCancelled = true;
		}

		FHttpResponsePtr GetResponse() const override
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			return Response;
		}

		void Complete()
		{
			FHttpResponsePtr CompletedResponse;
			bool bConnected = false;
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				if (bCancelled)
				{
					CompletedResponse = nullptr;
				}
				else
				{
					CompletedResponse = Response;
					bConnected = bConnectedSuccessfully;
				}
			}
			const FHttpRequestPtr Self{ std::shared_ptr<IHttpRequest>(shared_from_this()) };
			CompleteDelegate.ExecuteIfBound(Self, CompletedResponse, bConnected);
		}

	private:
		mutable std::mutex Mutex;
		FString Url;
		FString Verb = TEXT("GET");
		FString Content;
		TMap<FString, FString> Headers;
		FHttpRequestCompleteDelegate CompleteDelegate;
		FHttpResponsePtr Response;
		bool bConnectedSuccessfully = false;
		bool bCancelled = false;
	};
}

FHttpModule& FHttpModule::Get()
{
	static FHttpModule Instance;
	return Instance;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FHttpModule::CreateRequest()
{
	return TSharedRef<IHttpRequest, ESPMode::ThreadSafe>(std::shared_ptr<IHttpRequest>(std::make_shared<FShimHttpRequest>()));
}

void UEShim::SetFakeHttpHandler(FFakeHttpHandler NewHandler)
{
	std::lock_guard<std::mutex> Lock(HandlerMutex());
	Handler() = std::move(NewHandler);
}

bool UEShim::TickHttp()
{
	std::vector<FPendingCompletion> Ready;
	{
		std::lock_guard<std::mutex> Lock(PendingMutex());
		const auto Now = std::chrono::steady_clock::now();
		std::vector<FPendingCompletion>& Pending = PendingCompletions();
		for (auto It = Pending.begin(); It != Pending.end();)
		{
			if (It->ReadyTime <= Now)
			{
				Ready.push_back(std::move(*It));
				It = Pending.erase(It);
			}
			else
			{
				++It;
			}
		}
	}
	for (FPendingCompletion& Completion : Ready)
	{
		Completion.Request->Complete();
	}
	return !Ready.empty();
}

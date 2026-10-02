// Copyright (C) Microsoft Corporation. All rights reserved.

// Emulation of the Unreal "HTTP" module interfaces used by the GSDK, backed by an in-process fake server so tests can
// play the role of the PlayFab agent. Like Unreal, GetResponse() is available as soon as the response has arrived,
// while OnProcessRequestComplete() delegates run on the game thread (when it is pumped by the test).

#pragma once

#include "UEShim/Core.h"

class IHttpRequest;
class IHttpResponse;

typedef TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> FHttpRequestPtr;
typedef TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FHttpRequestRef;
typedef TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> FHttpResponsePtr;

DECLARE_DELEGATE_ThreeParams(FHttpRequestCompleteDelegate, FHttpRequestPtr, FHttpResponsePtr, bool);

class IHttpBase
{
public:
	virtual ~IHttpBase() = default;
	virtual FString GetURL() const = 0;
	virtual FString GetContentAsString() const = 0;
	virtual uint64 GetContentLength() const = 0;
};

class IHttpResponse : public IHttpBase
{
public:
	virtual int32 GetResponseCode() const = 0;
};

class IHttpRequest : public IHttpBase
{
public:
	virtual FString GetVerb() const = 0;
	virtual void SetURL(const FString& URL) = 0;
	virtual void SetVerb(const FString& Verb) = 0;
	virtual void SetHeader(const FString& HeaderName, const FString& HeaderValue) = 0;
	virtual void SetContentAsString(const FString& ContentString) = 0;
	virtual bool ProcessRequest() = 0;
	virtual void CancelRequest() = 0;
	virtual FHttpRequestCompleteDelegate& OnProcessRequestComplete() = 0;
	virtual FHttpResponsePtr GetResponse() const = 0;
};

class FHttpModule
{
public:
	static FHttpModule& Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateRequest();
};

namespace UEShim
{
	struct FFakeHttpResponse
	{
		int32 Code = 200;
		FString Body;
	};

	// Handles every request sent with ProcessRequest(), on the calling thread. Without a handler, requests fail to connect.
	typedef std::function<FFakeHttpResponse(const FString& Verb, const FString& Url, const FString& Body)> FFakeHttpHandler;
	void SetFakeHttpHandler(FFakeHttpHandler Handler);

	// Runs the completion delegates of requests that finished; called from the game thread pump.
	bool TickHttp();
}

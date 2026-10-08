// Copyright (C) Microsoft Corporation. All rights reserved.

// Minimal emulation of the Unreal Engine Core APIs used by the PlayFab GSDK plugin and its automation tests, so the
// unmodified plugin sources and tests compile and run with a plain C++ compiler (see UnrealPluginTests/README.md).
// This is NOT Unreal Engine: only what the GSDK uses is emulated, mirroring UE semantics where they matter
// (TCHAR is a wide character type, FString comparisons and TMap<FString, ...> lookups are case-insensitive,
// UE_LOG/FString::Printf require TEXT() format strings, MoveTemp rejects const objects and rvalues, ...).

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <future>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------------------------------------------------
// Basic types and macros

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;
typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;
typedef char ANSICHAR;
typedef wchar_t WIDECHAR;
typedef wchar_t TCHAR;
typedef std::size_t SIZE_T;
typedef decltype(nullptr) TYPE_OF_NULLPTR;

#define MAX_int32 ((int32)0x7fffffff)
#define MAX_uint32 ((uint32)0xffffffff)
#define UE_KINDA_SMALL_NUMBER (1.e-4f)

#define TEXT_PASTE(x) L ## x
#define TEXT(x) TEXT_PASTE(x)

#define FORCEINLINE inline

// UObject reflection markup is ignored.
#define UCLASS(...)
#define USTRUCT(...)
#define UENUM(...)
#define UPROPERTY(...)
#define UFUNCTION(...)
#define GENERATED_BODY(...)
#define GENERATED_UCLASS_BODY(...)
#define GENERATED_USTRUCT_BODY(...)

namespace UEShim
{
	[[noreturn]] void CheckFailed(const char* Expression, const char* File, int Line);
	std::string ToUtf8(const std::wstring& Wide);
	std::wstring FromUtf8(const char* Utf8, std::size_t Length);
}

#define UESHIM_CHECK(Expression) do { if (!(Expression)) { ::UEShim::CheckFailed(#Expression, __FILE__, __LINE__); } } while (false)

// ---------------------------------------------------------------------------------------------------------------------
// Templates/UnrealTemplate.h

template <typename T>
constexpr std::remove_reference_t<T>&& MoveTemp(T&& Obj) noexcept
{
	typedef std::remove_reference_t<T> CastType;
	static_assert(std::is_lvalue_reference_v<T>, "MoveTemp called on an rvalue");
	static_assert(!std::is_const_v<CastType>, "MoveTemp called on a const object");
	return static_cast<CastType&&>(Obj);
}

template <typename T>
constexpr T&& Forward(std::remove_reference_t<T>& Obj) noexcept
{
	return static_cast<T&&>(Obj);
}

template <typename T>
constexpr T&& Forward(std::remove_reference_t<T>&& Obj) noexcept
{
	return static_cast<T&&>(Obj);
}

template <typename T>
void Swap(T& A, T& B)
{
	std::swap(A, B);
}

// ---------------------------------------------------------------------------------------------------------------------
// Containers/UnrealString.h

namespace ESearchCase
{
	enum Type
	{
		CaseSensitive,
		IgnoreCase,
	};
}

class FString;

namespace UEShim
{
	template <typename T> struct TIsTCharArray : std::false_type {};
	template <std::size_t N> struct TIsTCharArray<TCHAR[N]> : std::true_type {};
	template <std::size_t N> struct TIsTCharArray<const TCHAR[N]> : std::true_type {};

	template <typename T>
	inline constexpr bool IsValidVariadicArg = std::is_arithmetic_v<T> || std::is_pointer_v<T> || std::is_enum_v<T> || std::is_null_pointer_v<T>;

	FString FormatString(const TCHAR* Format, ...);
}

class FString
{
public:
	FString() = default;
	FString(const FString&) = default;
	FString(FString&&) noexcept = default;
	FString& operator=(const FString&) = default;
	FString& operator=(FString&&) noexcept = default;

	FString(const TCHAR* Str) : Data(Str ? Str : L"") {}
	FString(const ANSICHAR* Str) : Data(Str ? UEShim::FromUtf8(Str, std::strlen(Str)) : std::wstring()) {}
	FString(int32 Count, const TCHAR* Str) : Data(Str, Count) {}
	explicit FString(std::wstring InData) : Data(std::move(InData)) {}

	FString& operator=(const TCHAR* Str)
	{
		Data = Str ? Str : L"";
		return *this;
	}

	FString& operator=(const ANSICHAR* Str)
	{
		return *this = FString(Str);
	}

	const TCHAR* operator*() const { return Data.c_str(); }
	TCHAR& operator[](int32 Index) { return Data[Index]; }
	const TCHAR& operator[](int32 Index) const { return Data[Index]; }

	int32 Len() const { return static_cast<int32>(Data.size()); }
	bool IsEmpty() const { return Data.empty(); }
	void Empty() { Data.clear(); }
	void Reset() { Data.clear(); }

	FString& Append(const FString& Other) { Data += Other.Data; return *this; }
	FString& AppendChar(TCHAR Char) { Data += Char; return *this; }
	FString& operator+=(const FString& Other) { Data += Other.Data; return *this; }
	FString& operator+=(const TCHAR* Other) { Data += Other; return *this; }
	FString& operator+=(TCHAR Char) { Data += Char; return *this; }

	bool Equals(const FString& Other, ESearchCase::Type SearchCase = ESearchCase::CaseSensitive) const;
	int32 Compare(const FString& Other, ESearchCase::Type SearchCase = ESearchCase::CaseSensitive) const;
	int32 Find(const FString& SubStr, ESearchCase::Type SearchCase = ESearchCase::IgnoreCase) const;
	bool Contains(const FString& SubStr, ESearchCase::Type SearchCase = ESearchCase::IgnoreCase) const { return Find(SubStr, SearchCase) != -1; }
	bool StartsWith(const FString& Prefix, ESearchCase::Type SearchCase = ESearchCase::IgnoreCase) const;
	bool EndsWith(const FString& Suffix, ESearchCase::Type SearchCase = ESearchCase::IgnoreCase) const;
	bool IsNumeric() const;
	FString ToLower() const;

	template <typename FmtType, typename... Types>
	static FString Printf(const FmtType& Format, Types... Args)
	{
		static_assert(UEShim::TIsTCharArray<FmtType>::value, "Formatting string must be a TCHAR array.");
		static_assert((UEShim::IsValidVariadicArg<Types> && ...), "Invalid argument(s) passed to FString::Printf");
		return UEShim::FormatString(Format, Args...);
	}

	static FString SanitizeFloat(double Value, int32 MinFractionalDigits = 1);
	static FString FromInt(int32 Value) { return FString(std::to_wstring(Value)); }

	// Shim-only accessor.
	const std::wstring& GetStdString() const { return Data; }

	// Like Unreal, FString equality and ordering ignore case.
	friend bool operator==(const FString& A, const FString& B) { return A.Equals(B, ESearchCase::IgnoreCase); }
	friend bool operator==(const FString& A, const TCHAR* B) { return A.Equals(FString(B), ESearchCase::IgnoreCase); }
	friend bool operator<(const FString& A, const FString& B) { return A.Compare(B, ESearchCase::IgnoreCase) < 0; }

	friend FString operator+(const FString& A, const FString& B) { return FString(A.Data + B.Data); }
	friend FString operator+(const FString& A, const TCHAR* B) { return FString(A.Data + B); }
	friend FString operator+(const TCHAR* A, const FString& B) { return FString(A + B.Data); }

private:
	std::wstring Data;
};

// ---------------------------------------------------------------------------------------------------------------------
// Containers/Array.h, Containers/Map.h

template <typename InElementType>
class TArray
{
public:
	typedef InElementType ElementType;

	TArray() = default;
	TArray(std::initializer_list<ElementType> Init) : Data(Init) {}

	int32 Num() const { return static_cast<int32>(Data.size()); }
	bool IsEmpty() const { return Data.empty(); }
	bool IsValidIndex(int32 Index) const { return Index >= 0 && Index < Num(); }

	ElementType& operator[](int32 Index)
	{
		UESHIM_CHECK(IsValidIndex(Index));
		return Data[Index];
	}

	const ElementType& operator[](int32 Index) const
	{
		UESHIM_CHECK(IsValidIndex(Index));
		return Data[Index];
	}

	int32 Add(const ElementType& Item)
	{
		Data.push_back(Item);
		return Num() - 1;
	}

	int32 Add(ElementType&& Item)
	{
		Data.push_back(std::move(Item));
		return Num() - 1;
	}

	template <typename... ArgsType>
	int32 Emplace(ArgsType&&... Args)
	{
		Data.emplace_back(Forward<ArgsType>(Args)...);
		return Num() - 1;
	}

	void Append(const TArray& Other) { Data.insert(Data.end(), Other.Data.begin(), Other.Data.end()); }

	void RemoveAt(int32 Index, int32 Count = 1)
	{
		UESHIM_CHECK(Count >= 0 && Index >= 0 && Index + Count <= Num());
		Data.erase(Data.begin() + Index, Data.begin() + Index + Count);
	}

	bool Contains(const ElementType& Item) const
	{
		for (const ElementType& Element : Data)
		{
			if (Element == Item)
			{
				return true;
			}
		}
		return false;
	}

	ElementType& Last()
	{
		UESHIM_CHECK(Num() > 0);
		return Data.back();
	}

	void Empty() { Data.clear(); }
	void Reset() { Data.clear(); }

	ElementType* GetData() { return Data.data(); }
	const ElementType* GetData() const { return Data.data(); }

	auto begin() { return Data.begin(); }
	auto end() { return Data.end(); }
	auto begin() const { return Data.begin(); }
	auto end() const { return Data.end(); }

private:
	std::vector<ElementType> Data;
};

template <typename KeyType, typename ValueType>
struct TPair
{
	TPair() = default;

	template <typename KeyArg, typename ValueArg>
	TPair(KeyArg&& InKey, ValueArg&& InValue) : Key(Forward<KeyArg>(InKey)), Value(Forward<ValueArg>(InValue)) {}

	KeyType Key;
	ValueType Value;
};

// Insertion-ordered map with linear lookups; keys are compared with operator== (case-insensitive for FString, as in UE).
template <typename KeyType, typename ValueType>
class TMap
{
public:
	typedef TPair<KeyType, ValueType> ElementType;

	ValueType& Add(const KeyType& Key, const ValueType& Value)
	{
		if (ValueType* Existing = Find(Key))
		{
			*Existing = Value;
			return *Existing;
		}
		Pairs.emplace_back(Key, Value);
		return Pairs.back().Value;
	}

	ValueType& Add(const ElementType& Pair) { return Add(Pair.Key, Pair.Value); }

	ValueType* Find(const KeyType& Key)
	{
		for (ElementType& Pair : Pairs)
		{
			if (Pair.Key == Key)
			{
				return &Pair.Value;
			}
		}
		return nullptr;
	}

	const ValueType* Find(const KeyType& Key) const
	{
		return const_cast<TMap*>(this)->Find(Key);
	}

	ValueType FindRef(const KeyType& Key) const
	{
		const ValueType* Found = Find(Key);
		return Found ? *Found : ValueType();
	}

	bool Contains(const KeyType& Key) const { return Find(Key) != nullptr; }

	ValueType& operator[](const KeyType& Key)
	{
		ValueType* Found = Find(Key);
		UESHIM_CHECK(Found != nullptr);
		return *Found;
	}

	const ValueType& operator[](const KeyType& Key) const
	{
		const ValueType* Found = Find(Key);
		UESHIM_CHECK(Found != nullptr);
		return *Found;
	}

	int32 Remove(const KeyType& Key)
	{
		for (auto It = Pairs.begin(); It != Pairs.end(); ++It)
		{
			if (It->Key == Key)
			{
				Pairs.erase(It);
				return 1;
			}
		}
		return 0;
	}

	int32 Num() const { return static_cast<int32>(Pairs.size()); }
	void Empty() { Pairs.clear(); }

	auto begin() { return Pairs.begin(); }
	auto end() { return Pairs.end(); }
	auto begin() const { return Pairs.begin(); }
	auto end() const { return Pairs.end(); }

private:
	std::vector<ElementType> Pairs;
};

// ---------------------------------------------------------------------------------------------------------------------
// Templates/SharedPointer.h, Templates/UniquePtr.h

enum class ESPMode : uint8
{
	NotThreadSafe = 0,
	ThreadSafe = 1,
};

template <typename ObjectType, ESPMode Mode = ESPMode::ThreadSafe> class TSharedRef;
template <typename ObjectType, ESPMode Mode = ESPMode::ThreadSafe> class TSharedPtr;

namespace UEShim
{
	template <typename ObjectType>
	struct TRawPtrProxy
	{
		ObjectType* Object;
	};
}

template <typename ObjectType>
UEShim::TRawPtrProxy<ObjectType> MakeShareable(ObjectType* InObject)
{
	return UEShim::TRawPtrProxy<ObjectType>{ InObject };
}

template <typename ObjectType, ESPMode Mode>
class TSharedRef
{
public:
	template <typename OtherType, typename = std::enable_if_t<std::is_convertible_v<OtherType*, ObjectType*>>>
	TSharedRef(const TSharedRef<OtherType, Mode>& Other) : Ptr(Other.Ptr) {}

	template <typename OtherType, typename = std::enable_if_t<std::is_convertible_v<OtherType*, ObjectType*>>>
	TSharedRef(const UEShim::TRawPtrProxy<OtherType>& Proxy) : Ptr(Proxy.Object)
	{
		UESHIM_CHECK(Ptr != nullptr);
	}

	explicit TSharedRef(std::shared_ptr<ObjectType> InPtr) : Ptr(std::move(InPtr))
	{
		UESHIM_CHECK(Ptr != nullptr);
	}

	ObjectType* operator->() const { return Ptr.get(); }
	ObjectType& operator*() const { return *Ptr; }
	ObjectType& Get() const { return *Ptr; }
	bool IsValid() const { return true; }
	TSharedPtr<ObjectType, Mode> ToSharedPtr() const { return TSharedPtr<ObjectType, Mode>(Ptr); }

	friend bool operator==(const TSharedRef& A, const TSharedRef& B) { return A.Ptr == B.Ptr; }

	std::shared_ptr<ObjectType> Ptr;
};

template <typename ObjectType, ESPMode Mode>
class TSharedPtr
{
public:
	TSharedPtr() = default;
	TSharedPtr(TYPE_OF_NULLPTR) {}

	template <typename OtherType, typename = std::enable_if_t<std::is_convertible_v<OtherType*, ObjectType*>>>
	TSharedPtr(const TSharedPtr<OtherType, Mode>& Other) : Ptr(Other.Ptr) {}

	template <typename OtherType, typename = std::enable_if_t<std::is_convertible_v<OtherType*, ObjectType*>>>
	TSharedPtr(const TSharedRef<OtherType, Mode>& Other) : Ptr(Other.Ptr) {}

	template <typename OtherType, typename = std::enable_if_t<std::is_convertible_v<OtherType*, ObjectType*>>>
	TSharedPtr(const UEShim::TRawPtrProxy<OtherType>& Proxy) : Ptr(Proxy.Object) {}

	explicit TSharedPtr(std::shared_ptr<ObjectType> InPtr) : Ptr(std::move(InPtr)) {}

	ObjectType* Get() const { return Ptr.get(); }
	bool IsValid() const { return Ptr != nullptr; }
	explicit operator bool() const { return IsValid(); }

	ObjectType* operator->() const
	{
		UESHIM_CHECK(IsValid());
		return Ptr.get();
	}

	ObjectType& operator*() const
	{
		UESHIM_CHECK(IsValid());
		return *Ptr;
	}

	TSharedRef<ObjectType, Mode> ToSharedRef() const
	{
		UESHIM_CHECK(IsValid());
		return TSharedRef<ObjectType, Mode>(Ptr);
	}

	void Reset() { Ptr.reset(); }

	friend bool operator==(const TSharedPtr& A, const TSharedPtr& B) { return A.Ptr == B.Ptr; }
	friend bool operator==(const TSharedPtr& A, TYPE_OF_NULLPTR) { return A.Ptr == nullptr; }

	std::shared_ptr<ObjectType> Ptr;
};

template <typename ObjectType, ESPMode Mode = ESPMode::ThreadSafe, typename... ArgTypes>
TSharedRef<ObjectType, Mode> MakeShared(ArgTypes&&... Args)
{
	return TSharedRef<ObjectType, Mode>(std::make_shared<ObjectType>(Forward<ArgTypes>(Args)...));
}

template <typename T>
class TUniquePtr
{
public:
	TUniquePtr() = default;
	TUniquePtr(TYPE_OF_NULLPTR) {}
	explicit TUniquePtr(T* InPtr) : Ptr(InPtr) {}
	TUniquePtr(TUniquePtr&&) noexcept = default;
	TUniquePtr& operator=(TUniquePtr&&) noexcept = default;

	template <typename OtherType, typename = std::enable_if_t<std::is_convertible_v<OtherType*, T*>>>
	TUniquePtr(TUniquePtr<OtherType>&& Other) : Ptr(Other.Release()) {}

	template <typename OtherType, typename = std::enable_if_t<std::is_convertible_v<OtherType*, T*>>>
	TUniquePtr& operator=(TUniquePtr<OtherType>&& Other)
	{
		Ptr.reset(Other.Release());
		return *this;
	}

	TUniquePtr& operator=(TYPE_OF_NULLPTR)
	{
		Ptr.reset();
		return *this;
	}

	T* Get() const { return Ptr.get(); }
	bool IsValid() const { return Ptr != nullptr; }
	explicit operator bool() const { return IsValid(); }

	T* operator->() const
	{
		UESHIM_CHECK(IsValid());
		return Ptr.get();
	}

	T& operator*() const
	{
		UESHIM_CHECK(IsValid());
		return *Ptr;
	}

	T* Release() { return Ptr.release(); }
	void Reset(T* InPtr = nullptr) { Ptr.reset(InPtr); }

private:
	std::unique_ptr<T> Ptr;
};

template <typename T, typename... ArgTypes>
TUniquePtr<T> MakeUnique(ArgTypes&&... Args)
{
	return TUniquePtr<T>(new T(Forward<ArgTypes>(Args)...));
}

// ---------------------------------------------------------------------------------------------------------------------
// Templates/Function.h

template <typename FuncType> class TFunction;
template <typename FuncType> class TUniqueFunction;

namespace UEShim
{
	template <typename T> struct TIsTFunction : std::false_type {};
	template <typename F> struct TIsTFunction<TFunction<F>> : std::true_type {};
	template <typename F> struct TIsTFunction<TUniqueFunction<F>> : std::true_type {};
}

// Copyable type-erased callable; like UE, the bound callable must be copyable.
template <typename Ret, typename... ParamTypes>
class TFunction<Ret(ParamTypes...)>
{
public:
	TFunction(TYPE_OF_NULLPTR = nullptr) {}

	template <typename FunctorType, typename = std::enable_if_t<!UEShim::TIsTFunction<std::decay_t<FunctorType>>::value && std::is_invocable_r_v<Ret, std::decay_t<FunctorType>&, ParamTypes...>>>
	TFunction(FunctorType&& InFunc) : Func(Forward<FunctorType>(InFunc)) {}

	Ret operator()(ParamTypes... Params) const
	{
		UESHIM_CHECK(static_cast<bool>(Func));
		return Func(Forward<ParamTypes>(Params)...);
	}

	explicit operator bool() const { return static_cast<bool>(Func); }
	bool IsSet() const { return static_cast<bool>(Func); }
	void Reset() { Func = nullptr; }

private:
	std::function<Ret(ParamTypes...)> Func;
};

// Move-only type-erased callable.
template <typename Ret, typename... ParamTypes>
class TUniqueFunction<Ret(ParamTypes...)>
{
public:
	TUniqueFunction(TYPE_OF_NULLPTR = nullptr) {}

	template <typename FunctorType, typename = std::enable_if_t<!std::is_same_v<std::decay_t<FunctorType>, TUniqueFunction> && std::is_invocable_r_v<Ret, std::decay_t<FunctorType>&, ParamTypes...>>>
	TUniqueFunction(FunctorType&& InFunc) : Callable(std::make_unique<TCallable<std::decay_t<FunctorType>>>(Forward<FunctorType>(InFunc))) {}

	TUniqueFunction(TUniqueFunction&&) noexcept = default;
	TUniqueFunction& operator=(TUniqueFunction&&) noexcept = default;
	TUniqueFunction(const TUniqueFunction&) = delete;
	TUniqueFunction& operator=(const TUniqueFunction&) = delete;

	Ret operator()(ParamTypes... Params) const
	{
		UESHIM_CHECK(Callable != nullptr);
		return Callable->Call(Forward<ParamTypes>(Params)...);
	}

	explicit operator bool() const { return Callable != nullptr; }
	bool IsSet() const { return Callable != nullptr; }
	void Reset() { Callable.reset(); }

private:
	struct ICallable
	{
		virtual ~ICallable() = default;
		virtual Ret Call(ParamTypes... Params) = 0;
	};

	template <typename FunctorType>
	struct TCallable final : ICallable
	{
		template <typename ArgType>
		explicit TCallable(ArgType&& InFunctor) : Functor(Forward<ArgType>(InFunctor)) {}

		Ret Call(ParamTypes... Params) override
		{
			return std::invoke(Functor, Forward<ParamTypes>(Params)...);
		}

		FunctorType Functor;
	};

	std::unique_ptr<ICallable> Callable;
};

// ---------------------------------------------------------------------------------------------------------------------
// Delegates/Delegate.h (single-cast only)

template <typename FuncType> class TDelegate;

template <typename Ret, typename... ParamTypes>
class TDelegate<Ret(ParamTypes...)>
{
public:
	template <typename FunctorType>
	void BindLambda(FunctorType&& InFunctor)
	{
		Func = Forward<FunctorType>(InFunctor);
	}

	template <typename FunctorType>
	static TDelegate CreateLambda(FunctorType&& InFunctor)
	{
		TDelegate Result;
		Result.BindLambda(Forward<FunctorType>(InFunctor));
		return Result;
	}

	template <typename UserClass>
	void BindRaw(UserClass* Object, Ret (UserClass::*Method)(ParamTypes...))
	{
		Func = [Object, Method](ParamTypes... Params) -> Ret { return (Object->*Method)(Forward<ParamTypes>(Params)...); };
	}

	bool IsBound() const { return static_cast<bool>(Func); }
	void Unbind() { Func = nullptr; }

	Ret Execute(ParamTypes... Params) const
	{
		UESHIM_CHECK(IsBound());
		return Func(Forward<ParamTypes>(Params)...);
	}

	bool ExecuteIfBound(ParamTypes... Params) const requires std::is_void_v<Ret>
	{
		if (!IsBound())
		{
			return false;
		}
		Func(Forward<ParamTypes>(Params)...);
		return true;
	}

private:
	std::function<Ret(ParamTypes...)> Func;
};

#define DECLARE_DELEGATE(DelegateName) typedef TDelegate<void()> DelegateName;
#define DECLARE_DELEGATE_RetVal(RetValType, DelegateName) typedef TDelegate<RetValType()> DelegateName;
#define DECLARE_DELEGATE_OneParam(DelegateName, Param1Type) typedef TDelegate<void(Param1Type)> DelegateName;
#define DECLARE_DELEGATE_TwoParams(DelegateName, Param1Type, Param2Type) typedef TDelegate<void(Param1Type, Param2Type)> DelegateName;
#define DECLARE_DELEGATE_ThreeParams(DelegateName, Param1Type, Param2Type, Param3Type) typedef TDelegate<void(Param1Type, Param2Type, Param3Type)> DelegateName;

// Dynamic (UObject) delegates are emulated with plain delegates; BindDynamic is not supported.
#define DECLARE_DYNAMIC_DELEGATE(DelegateName) class DelegateName : public TDelegate<void()> {}
#define DECLARE_DYNAMIC_DELEGATE_RetVal(RetValType, DelegateName) class DelegateName : public TDelegate<RetValType()> {}
#define DECLARE_DYNAMIC_DELEGATE_OneParam(DelegateName, Param1Type, Param1Name) class DelegateName : public TDelegate<void(Param1Type)> {}

// ---------------------------------------------------------------------------------------------------------------------
// Logging/LogMacros.h, Misc/OutputDevice.h

namespace ELogVerbosity
{
	enum Type : uint8
	{
		NoLogging = 0,
		Fatal,
		Error,
		Warning,
		Display,
		Log,
		Verbose,
		VeryVerbose,
		All = VeryVerbose,
	};
}

struct FLogCategoryBase
{
	explicit FLogCategoryBase(const TCHAR* InName) : Name(InName) {}
	const TCHAR* Name;
};

#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity, CompileTimeVerbosity) extern FLogCategoryBase CategoryName;
#define DEFINE_LOG_CATEGORY(CategoryName) FLogCategoryBase CategoryName(TEXT(#CategoryName));
#define DEFINE_LOG_CATEGORY_STATIC(CategoryName, DefaultVerbosity, CompileTimeVerbosity) static FLogCategoryBase CategoryName(TEXT(#CategoryName));

namespace UEShim
{
	void LogMessage(const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FString& Message);

	template <typename FmtType, typename... Types>
	void LogFormatted(const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FmtType& Format, Types... Args)
	{
		static_assert(TIsTCharArray<FmtType>::value, "Formatting string must be a TCHAR array.");
		static_assert((IsValidVariadicArg<Types> && ...), "Invalid argument(s) passed to UE_LOG");
		LogMessage(Category, Verbosity, FormatString(Format, Args...));
	}
}

#define UE_LOG(CategoryName, Verbosity, ...) ::UEShim::LogFormatted(CategoryName, ELogVerbosity::Verbosity, __VA_ARGS__)

class FOutputDevice
{
public:
	virtual ~FOutputDevice() = default;
	virtual void Serialize(const TCHAR* Message, ELogVerbosity::Type Verbosity, const TCHAR* Category) = 0;
};

// Records its file name but writes nothing, so tests don't create log files in arbitrary locations.
class FOutputDeviceFile : public FOutputDevice
{
public:
	explicit FOutputDeviceFile(const TCHAR* InFilename = nullptr) : Filename(InFilename) {}
	void Serialize(const TCHAR*, ELogVerbosity::Type, const TCHAR*) override {}
	const FString& GetFilename() const { return Filename; }

private:
	FString Filename;
};

class FOutputDeviceRedirector
{
public:
	void AddOutputDevice(FOutputDevice* Device);
	void RemoveOutputDevice(FOutputDevice* Device);
	void Broadcast(const TCHAR* Message, ELogVerbosity::Type Verbosity, const TCHAR* Category);

private:
	std::mutex Mutex;
	std::vector<FOutputDevice*> Devices;
};

extern FOutputDeviceRedirector* GLog;

// ---------------------------------------------------------------------------------------------------------------------
// Math, time

struct FMath
{
	template <typename T> static constexpr T Max(const T A, const T B) { return (A >= B) ? A : B; }
	template <typename T> static constexpr T Min(const T A, const T B) { return (A <= B) ? A : B; }
	template <typename T> static constexpr T Clamp(const T X, const T MinValue, const T MaxValue) { return X < MinValue ? MinValue : (X < MaxValue ? X : MaxValue); }
	template <typename T> static constexpr T Abs(const T A) { return (A < T(0)) ? -A : A; }
};

namespace ETimespan
{
	inline constexpr int64 TicksPerMillisecond = 10000;
	inline constexpr int64 TicksPerSecond = 10000000;
	inline constexpr int64 TicksPerMinute = 600000000;
	inline constexpr int64 TicksPerHour = 36000000000;
	inline constexpr int64 TicksPerDay = 864000000000;
}

class FTimespan
{
public:
	FTimespan() = default;
	explicit FTimespan(int64 InTicks) : Ticks(InTicks) {}
	FTimespan(int32 Hours, int32 Minutes, int32 Seconds)
		: Ticks(Hours * ETimespan::TicksPerHour + Minutes * ETimespan::TicksPerMinute + Seconds * ETimespan::TicksPerSecond) {}

	static FTimespan FromMilliseconds(double Milliseconds) { return FTimespan(static_cast<int64>(Milliseconds * ETimespan::TicksPerMillisecond)); }
	static FTimespan FromSeconds(double Seconds) { return FTimespan(static_cast<int64>(Seconds * ETimespan::TicksPerSecond)); }

	int64 GetTicks() const { return Ticks; }
	bool IsZero() const { return Ticks == 0; }
	double GetTotalMilliseconds() const { return static_cast<double>(Ticks) / ETimespan::TicksPerMillisecond; }
	double GetTotalSeconds() const { return static_cast<double>(Ticks) / ETimespan::TicksPerSecond; }

	friend bool operator==(const FTimespan& A, const FTimespan& B) { return A.Ticks == B.Ticks; }
	friend bool operator<(const FTimespan& A, const FTimespan& B) { return A.Ticks < B.Ticks; }

private:
	int64 Ticks = 0;
};

class FDateTime
{
public:
	FDateTime() = default;
	FDateTime(int64 InTicks) : Ticks(InTicks) {}
	FDateTime(int32 Year, int32 Month, int32 Day, int32 Hour = 0, int32 Minute = 0, int32 Second = 0, int32 Millisecond = 0);

	static FDateTime Now();
	static FDateTime UtcNow();
	static bool ParseIso8601(const TCHAR* DateTimeString, FDateTime& OutDateTime);

	int64 GetTicks() const { return Ticks; }
	int64 ToUnixTimestamp() const;

	FTimespan operator-(const FDateTime& Other) const { return FTimespan(Ticks - Other.Ticks); }
	FDateTime operator+(const FTimespan& Span) const { return FDateTime(Ticks + Span.GetTicks()); }
	friend bool operator==(const FDateTime& A, const FDateTime& B) { return A.Ticks == B.Ticks; }
	friend bool operator<(const FDateTime& A, const FDateTime& B) { return A.Ticks < B.Ticks; }

private:
	int64 Ticks = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// HAL: synchronization

class FCriticalSection
{
public:
	void Lock() { Mutex.lock(); }
	void Unlock() { Mutex.unlock(); }
	bool TryLock() { return Mutex.try_lock(); }

private:
	std::recursive_mutex Mutex;
};

class FScopeLock
{
public:
	explicit FScopeLock(FCriticalSection* InSyncObject) : SyncObject(InSyncObject)
	{
		SyncObject->Lock();
	}

	~FScopeLock()
	{
		SyncObject->Unlock();
	}

	FScopeLock(const FScopeLock&) = delete;
	FScopeLock& operator=(const FScopeLock&) = delete;

private:
	FCriticalSection* SyncObject;
};

class FEvent
{
public:
	explicit FEvent(bool bInIsManualReset) : bIsManualReset(bInIsManualReset) {}

	void Trigger();
	void Reset();
	// Returns true if the event was triggered within the timeout; auto-reset events are reset by a successful wait.
	bool Wait(uint32 WaitTimeMs, const bool bIgnoreThreadIdleStats = false);
	bool Wait() { return Wait(MAX_uint32); }
	bool Wait(const FTimespan& WaitTime, const bool bIgnoreThreadIdleStats = false)
	{
		return Wait(static_cast<uint32>(FMath::Clamp(WaitTime.GetTotalMilliseconds(), 0.0, static_cast<double>(MAX_uint32 - 1))), bIgnoreThreadIdleStats);
	}
	bool IsManualReset() const { return bIsManualReset; }

private:
	std::mutex Mutex;
	std::condition_variable Condition;
	bool bTriggered = false;
	const bool bIsManualReset;
};

enum class EEventMode
{
	AutoReset,
	ManualReset,
};

class FEventRef
{
public:
	explicit FEventRef(EEventMode Mode = EEventMode::AutoReset) : Event(new FEvent(Mode == EEventMode::ManualReset)) {}
	~FEventRef() { delete Event; }
	FEventRef(const FEventRef&) = delete;
	FEventRef& operator=(const FEventRef&) = delete;

	FEvent* operator->() const { return Event; }
	FEvent& operator*() const { return *Event; }
	FEvent* Get() const { return Event; }

private:
	FEvent* Event;
};

template <typename T>
class TAtomic
{
public:
	TAtomic() : Value(T()) {}
	TAtomic(T InValue) : Value(InValue) {}
	TAtomic(const TAtomic&) = delete;
	TAtomic& operator=(const TAtomic&) = delete;

	operator T() const { return Value.load(); }
	T operator=(T InValue)
	{
		Value.store(InValue);
		return InValue;
	}
	T Load() const { return Value.load(); }
	void Store(T InValue) { Value.store(InValue); }

private:
	std::atomic<T> Value;
};

// ---------------------------------------------------------------------------------------------------------------------
// Async: Async/Async.h, Async/Future.h, Async/TaskGraphInterfaces.h
//
// The thread that starts the process is the "game thread". AsyncTask(GameThread) queues work that only runs when that
// thread calls FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread), like a game thread that is
// busy until its next tick. Other named threads run on a background worker; EAsyncExecution::Thread starts a new thread.

namespace ENamedThreads
{
	enum Type : int32
	{
		GameThread = 1,
		AnyThread = 0xff,
		AnyHiPriThreadNormalTask,
		AnyBackgroundThreadNormalTask,
	};
}

enum class EAsyncExecution
{
	TaskGraph,
	TaskGraphMainThread,
	Thread,
	ThreadIfForkSafe,
	ThreadPool,
	LargeThreadPool,
};

template <typename ResultType> class TFuture;

template <>
class TFuture<void>
{
public:
	TFuture() = default;
	explicit TFuture(std::shared_future<void> InFuture) : Future(std::move(InFuture)) {}
	TFuture(TFuture&&) noexcept = default;
	TFuture& operator=(TFuture&&) noexcept = default;
	TFuture(const TFuture&) = delete;
	TFuture& operator=(const TFuture&) = delete;

	bool IsValid() const { return Future.valid(); }
	bool IsReady() const { return IsValid() && Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }

	void Wait() const
	{
		if (IsValid())
		{
			Future.wait();
		}
	}

	bool WaitFor(const FTimespan& Duration) const
	{
		return IsValid() && Future.wait_for(std::chrono::microseconds(Duration.GetTicks() / 10)) == std::future_status::ready;
	}

private:
	std::shared_future<void> Future;
};

void AsyncTask(ENamedThreads::Type Thread, TUniqueFunction<void()> Function);

namespace UEShim
{
	TFuture<void> LaunchAsync(EAsyncExecution Execution, TUniqueFunction<void()> Function);
}

template <typename CallableType>
TFuture<void> Async(EAsyncExecution Execution, CallableType&& Callable, TUniqueFunction<void()> CompletionCallback = nullptr)
{
	static_assert(std::is_void_v<std::invoke_result_t<std::decay_t<CallableType>&>>, "The UE shim only implements Async for callables returning void");
	return UEShim::LaunchAsync(Execution,
		[Function = TUniqueFunction<void()>(Forward<CallableType>(Callable)), Completion = MoveTemp(CompletionCallback)]() mutable
		{
			Function();
			if (Completion)
			{
				Completion();
			}
		});
}

class FTaskGraphInterface
{
public:
	static FTaskGraphInterface& Get();
	// Runs queued game thread tasks (and completed HTTP request callbacks) until there are none left.
	void ProcessThreadUntilIdle(ENamedThreads::Type CurrentThread);
};

bool IsInGameThread();

// ---------------------------------------------------------------------------------------------------------------------
// Platform, paths, files, command line

struct FPlatformMisc
{
	static FString GetEnvironmentVariable(const TCHAR* VariableName);
	static void SetEnvironmentVar(const TCHAR* VariableName, const TCHAR* Value);
};

struct FPlatformProcess
{
	// Events are never freed, mirroring UE's event pool where returned events stay valid.
	static FEvent* GetSynchEventFromPool(bool bIsManualReset = false);
	static void ReturnSynchEventToPool(FEvent* Event);
	static void Sleep(float Seconds);
};

struct FPaths
{
	static bool FileExists(const FString& Path);
	static bool DirectoryExists(const FString& Path);
	// A per-process scratch directory, with a trailing slash.
	static FString ProjectDir();

	template <typename... PathTypes>
	static FString Combine(const PathTypes&... Paths)
	{
		FString Result;
		(AppendPath(Result, FString(Paths)), ...);
		return Result;
	}

private:
	static void AppendPath(FString& Path, const FString& Part);
};

class IPlatformFile
{
public:
	bool FileExists(const TCHAR* Filename);
	bool DirectoryExists(const TCHAR* Directory);
	bool CreateDirectoryTree(const TCHAR* Directory);
	bool DeleteFile(const TCHAR* Filename);
};

class FPlatformFileManager
{
public:
	static FPlatformFileManager& Get();
	IPlatformFile& GetPlatformFile() { return PlatformFile; }

private:
	IPlatformFile PlatformFile;
};

struct FFileHelper
{
	enum class EEncodingOptions
	{
		AutoDetect,
		ForceAnsi,
		ForceUnicode,
		ForceUTF8,
		ForceUTF8WithoutBOM,
	};

	static bool LoadFileToString(FString& Result, const TCHAR* Filename);
	static bool SaveStringToFile(const FString& String, const TCHAR* Filename, EEncodingOptions EncodingOptions = EEncodingOptions::AutoDetect);
};

struct FCommandLine
{
	static const TCHAR* Get();
	static void Set(const TCHAR* NewCommandLine);
};

struct FParse
{
	// True if the stream contains the switch "-Param" (or "/Param"), ignoring case.
	static bool Param(const TCHAR* Stream, const TCHAR* Param);
};

// ---------------------------------------------------------------------------------------------------------------------
// Modules/ModuleManager.h

class IModuleInterface
{
public:
	virtual ~IModuleInterface() = default;
	virtual void StartupModule() {}
	virtual void ShutdownModule() {}
};

class FModuleManager
{
public:
	typedef IModuleInterface* (*FModuleFactory)();

	static FModuleManager& Get();

	template <typename TModuleInterface>
	static TModuleInterface& LoadModuleChecked(const TCHAR* ModuleName)
	{
		return static_cast<TModuleInterface&>(Get().LoadModuleCheckedInternal(ModuleName));
	}

	void RegisterModule(const TCHAR* ModuleName, FModuleFactory Factory);
	// Calls ShutdownModule and destroys every loaded module, like engine shutdown.
	void UnloadModulesAtShutdown();

private:
	IModuleInterface& LoadModuleCheckedInternal(const TCHAR* ModuleName);

	struct FModuleEntry
	{
		FString Name;
		FModuleFactory Factory = nullptr;
		std::unique_ptr<IModuleInterface> Module;
	};

	std::recursive_mutex Mutex;
	std::vector<FModuleEntry> Modules;
};

namespace UEShim
{
	struct FModuleRegistrar
	{
		FModuleRegistrar(const TCHAR* ModuleName, FModuleManager::FModuleFactory Factory)
		{
			FModuleManager::Get().RegisterModule(ModuleName, Factory);
		}
	};
}

#define IMPLEMENT_MODULE(ModuleImplClass, ModuleName) \
	static ::UEShim::FModuleRegistrar GUEShimModuleRegistrar_##ModuleName(TEXT(#ModuleName), []() -> IModuleInterface* { return new ModuleImplClass(); });

// ---------------------------------------------------------------------------------------------------------------------
// The few Engine/CoreUObject bits the plugin touches

class UObject
{
};

class UBlueprintFunctionLibrary : public UObject
{
};

struct FUrlConfig
{
	int32 DefaultPort = 7777;
};

struct FURL
{
	static FUrlConfig UrlConfig;
};

namespace UEShim
{
	// Shuts the emulated engine down: unloads modules and stops background threads. Called by the test runner.
	void ShutdownEngine();
}

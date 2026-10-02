// Copyright (C) Microsoft Corporation. All rights reserved.

// Emulation of the Unreal "Json" module DOM (Dom/JsonObject.h, Dom/JsonValue.h, Serialization/JsonSerializer.h).
// Type mismatches log the same errors as Unreal (e.g. "Json Value of type 'Object' used as a 'String'.").

#pragma once

#include "UEShim/Core.h"

enum class EJson
{
	None,
	Null,
	String,
	Number,
	Boolean,
	Array,
	Object,
};

class FJsonObject;

class FJsonValue
{
public:
	virtual ~FJsonValue() = default;

	virtual bool TryGetString(FString& OutString) const { return false; }
	virtual bool TryGetNumber(double& OutNumber) const { return false; }
	virtual bool TryGetBool(bool& OutBool) const { return false; }
	virtual bool TryGetArray(const TArray<TSharedPtr<FJsonValue>>*& OutArray) const { return false; }
	virtual bool TryGetObject(const TSharedPtr<FJsonObject>*& OutObject) const { return false; }
	bool TryGetNumber(int32& OutNumber) const;
	bool TryGetNumber(int64& OutNumber) const;

	FString AsString() const;
	double AsNumber() const;
	bool AsBool() const;
	const TArray<TSharedPtr<FJsonValue>>& AsArray() const;
	const TSharedPtr<FJsonObject>& AsObject() const;
	bool IsNull() const { return Type == EJson::Null || Type == EJson::None; }

	EJson Type = EJson::None;

protected:
	virtual FString GetType() const = 0;
	void ErrorMessage(const TCHAR* InType) const;
};

class FJsonValueString : public FJsonValue
{
public:
	explicit FJsonValueString(const FString& InValue) : Value(InValue) { Type = EJson::String; }
	bool TryGetString(FString& OutString) const override { OutString = Value; return true; }
	bool TryGetNumber(double& OutNumber) const override;
	bool TryGetBool(bool& OutBool) const override;

protected:
	FString GetType() const override { return TEXT("String"); }
	FString Value;
};

class FJsonValueNumber : public FJsonValue
{
public:
	explicit FJsonValueNumber(double InValue) : Value(InValue) { Type = EJson::Number; }
	bool TryGetNumber(double& OutNumber) const override { OutNumber = Value; return true; }
	bool TryGetString(FString& OutString) const override;
	bool TryGetBool(bool& OutBool) const override { OutBool = Value != 0.0; return true; }

protected:
	FString GetType() const override { return TEXT("Number"); }
	double Value;
};

class FJsonValueBoolean : public FJsonValue
{
public:
	explicit FJsonValueBoolean(bool InValue) : Value(InValue) { Type = EJson::Boolean; }
	bool TryGetBool(bool& OutBool) const override { OutBool = Value; return true; }
	bool TryGetNumber(double& OutNumber) const override { OutNumber = Value ? 1.0 : 0.0; return true; }
	bool TryGetString(FString& OutString) const override { OutString = Value ? TEXT("true") : TEXT("false"); return true; }

protected:
	FString GetType() const override { return TEXT("Boolean"); }
	bool Value;
};

class FJsonValueArray : public FJsonValue
{
public:
	explicit FJsonValueArray(const TArray<TSharedPtr<FJsonValue>>& InValue) : Value(InValue) { Type = EJson::Array; }
	bool TryGetArray(const TArray<TSharedPtr<FJsonValue>>*& OutArray) const override { OutArray = &Value; return true; }

protected:
	FString GetType() const override { return TEXT("Array"); }
	TArray<TSharedPtr<FJsonValue>> Value;
};

class FJsonValueObject : public FJsonValue
{
public:
	explicit FJsonValueObject(TSharedPtr<FJsonObject> InValue) : Value(InValue) { Type = EJson::Object; }
	bool TryGetObject(const TSharedPtr<FJsonObject>*& OutObject) const override { OutObject = &Value; return true; }

protected:
	FString GetType() const override { return TEXT("Object"); }
	TSharedPtr<FJsonObject> Value;
};

class FJsonValueNull : public FJsonValue
{
public:
	FJsonValueNull() { Type = EJson::Null; }

protected:
	FString GetType() const override { return TEXT("Null"); }
};

class FJsonObject
{
public:
	TMap<FString, TSharedPtr<FJsonValue>> Values;

	bool HasField(const FString& FieldName) const;
	// Returns the field, or a null value (and logs a warning) if it is missing or not of the requested type.
	TSharedPtr<FJsonValue> GetField(const FString& FieldName, EJson JsonType = EJson::None) const;

	FString GetStringField(const FString& FieldName) const { return GetField(FieldName)->AsString(); }
	double GetNumberField(const FString& FieldName) const { return GetField(FieldName)->AsNumber(); }
	int32 GetIntegerField(const FString& FieldName) const;
	bool GetBoolField(const FString& FieldName) const { return GetField(FieldName)->AsBool(); }
	const TArray<TSharedPtr<FJsonValue>>& GetArrayField(const FString& FieldName) const;
	const TSharedPtr<FJsonObject>& GetObjectField(const FString& FieldName) const;

	bool TryGetStringField(const FString& FieldName, FString& OutString) const;

	void SetField(const FString& FieldName, const TSharedPtr<FJsonValue>& Value) { Values.Add(FieldName, Value); }
	void SetStringField(const FString& FieldName, const FString& Value) { SetField(FieldName, MakeShared<FJsonValueString>(Value)); }
	void SetNumberField(const FString& FieldName, double Value) { SetField(FieldName, MakeShared<FJsonValueNumber>(Value)); }
	void SetBoolField(const FString& FieldName, bool Value) { SetField(FieldName, MakeShared<FJsonValueBoolean>(Value)); }
	void SetArrayField(const FString& FieldName, const TArray<TSharedPtr<FJsonValue>>& Array) { SetField(FieldName, MakeShared<FJsonValueArray>(Array)); }
	void SetObjectField(const FString& FieldName, const TSharedPtr<FJsonObject>& Object) { SetField(FieldName, MakeShared<FJsonValueObject>(Object)); }
	void RemoveField(const FString& FieldName) { Values.Remove(FieldName); }
};

template <class CharType>
struct TPrettyJsonPrintPolicy
{
	static constexpr bool bPretty = true;
};

template <class CharType>
struct TCondensedJsonPrintPolicy
{
	static constexpr bool bPretty = false;
};

template <class CharType = TCHAR>
class TJsonReader
{
public:
	explicit TJsonReader(const FString& InContent) : Content(InContent) {}
	const FString& GetContent() const { return Content; }

private:
	FString Content;
};

template <class CharType = TCHAR>
class TJsonReaderFactory
{
public:
	static TSharedRef<TJsonReader<CharType>> Create(const FString& JsonString)
	{
		return MakeShared<TJsonReader<CharType>>(JsonString);
	}
};

template <class CharType = TCHAR, class PrintPolicy = TPrettyJsonPrintPolicy<CharType>>
class TJsonWriter
{
public:
	explicit TJsonWriter(FString* InOutput) : Output(InOutput) {}
	FString* GetOutput() const { return Output; }
	static constexpr bool IsPretty() { return PrintPolicy::bPretty; }

private:
	FString* Output;
};

template <class CharType = TCHAR, class PrintPolicy = TPrettyJsonPrintPolicy<CharType>>
class TJsonWriterFactory
{
public:
	static TSharedRef<TJsonWriter<CharType, PrintPolicy>> Create(FString* const Stream)
	{
		return MakeShared<TJsonWriter<CharType, PrintPolicy>>(Stream);
	}
};

namespace UEShim
{
	bool ParseJson(const FString& Content, TSharedPtr<FJsonValue>& OutValue);
	FString WriteJson(const FJsonObject& Object, bool bPretty);
}

class FJsonSerializer
{
public:
	template <class CharType>
	static bool Deserialize(const TSharedRef<TJsonReader<CharType>>& Reader, TSharedPtr<FJsonObject>& OutObject)
	{
		TSharedPtr<FJsonValue> Value;
		const TSharedPtr<FJsonObject>* Object = nullptr;
		if (!UEShim::ParseJson(Reader->GetContent(), Value) || !Value->TryGetObject(Object))
		{
			return false;
		}
		OutObject = *Object;
		return true;
	}

	template <class CharType>
	static bool Deserialize(const TSharedRef<TJsonReader<CharType>>& Reader, TSharedPtr<FJsonValue>& OutValue)
	{
		return UEShim::ParseJson(Reader->GetContent(), OutValue);
	}

	template <class CharType, class PrintPolicy>
	static bool Serialize(const TSharedRef<FJsonObject>& Object, const TSharedRef<TJsonWriter<CharType, PrintPolicy>>& Writer)
	{
		*Writer->GetOutput() += UEShim::WriteJson(*Object, TJsonWriter<CharType, PrintPolicy>::IsPretty());
		return true;
	}
};

// Copyright (C) Microsoft Corporation. All rights reserved.

#include <cmath>
#include <cstdio>
#include <cwchar>
#include <string>

#include "ShimInternal.h"

DEFINE_LOG_CATEGORY_STATIC(LogJson, Log, All);

namespace
{
	const TArray<TSharedPtr<FJsonValue>>& EmptyArray()
	{
		static const TArray<TSharedPtr<FJsonValue>>* Array = new TArray<TSharedPtr<FJsonValue>>();
		return *Array;
	}

	// Like Unreal, type mismatches return a valid empty object rather than null.
	const TSharedPtr<FJsonObject>& EmptyObject()
	{
		static const TSharedPtr<FJsonObject>* Object = new TSharedPtr<FJsonObject>(MakeShared<FJsonObject>());
		return *Object;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// FJsonValue

void FJsonValue::ErrorMessage(const TCHAR* InType) const
{
	UE_LOG(LogJson, Error, TEXT("Json Value of type '%s' used as a '%s'."), *GetType(), InType);
}

bool FJsonValue::TryGetNumber(int32& OutNumber) const
{
	double Number = 0.0;
	if (TryGetNumber(Number) && Number >= -2147483648.0 && Number <= 2147483647.0)
	{
		OutNumber = static_cast<int32>(std::llround(Number));
		return true;
	}
	return false;
}

bool FJsonValue::TryGetNumber(int64& OutNumber) const
{
	double Number = 0.0;
	if (TryGetNumber(Number))
	{
		OutNumber = static_cast<int64>(std::llround(Number));
		return true;
	}
	return false;
}

FString FJsonValue::AsString() const
{
	FString String;
	if (!TryGetString(String))
	{
		ErrorMessage(TEXT("String"));
	}
	return String;
}

double FJsonValue::AsNumber() const
{
	double Number = 0.0;
	if (!TryGetNumber(Number))
	{
		ErrorMessage(TEXT("Number"));
	}
	return Number;
}

bool FJsonValue::AsBool() const
{
	bool Bool = false;
	if (!TryGetBool(Bool))
	{
		ErrorMessage(TEXT("Boolean"));
	}
	return Bool;
}

const TArray<TSharedPtr<FJsonValue>>& FJsonValue::AsArray() const
{
	const TArray<TSharedPtr<FJsonValue>>* Array = &EmptyArray();
	if (!TryGetArray(Array))
	{
		ErrorMessage(TEXT("Array"));
	}
	return *Array;
}

const TSharedPtr<FJsonObject>& FJsonValue::AsObject() const
{
	const TSharedPtr<FJsonObject>* Object = &EmptyObject();
	if (!TryGetObject(Object))
	{
		ErrorMessage(TEXT("Object"));
	}
	return *Object;
}

bool FJsonValueString::TryGetNumber(double& OutNumber) const
{
	if (Value.IsNumeric())
	{
		OutNumber = std::wcstod(*Value, nullptr);
		return true;
	}
	return false;
}

bool FJsonValueString::TryGetBool(bool& OutBool) const
{
	if (Value.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("false"), ESearchCase::IgnoreCase))
	{
		OutBool = Value.Equals(TEXT("true"), ESearchCase::IgnoreCase);
		return true;
	}
	return false;
}

bool FJsonValueNumber::TryGetString(FString& OutString) const
{
	OutString = FString::SanitizeFloat(Value, 0);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// FJsonObject

bool FJsonObject::HasField(const FString& FieldName) const
{
	const TSharedPtr<FJsonValue>* Field = Values.Find(FieldName);
	return Field != nullptr && Field->IsValid();
}

TSharedPtr<FJsonValue> FJsonObject::GetField(const FString& FieldName, EJson JsonType) const
{
	const TSharedPtr<FJsonValue>* Field = Values.Find(FieldName);
	if (Field != nullptr && Field->IsValid())
	{
		if (JsonType == EJson::None || (*Field)->Type == JsonType)
		{
			return *Field;
		}
		UE_LOG(LogJson, Warning, TEXT("Field %s is of the wrong type."), *FieldName);
	}
	else
	{
		UE_LOG(LogJson, Warning, TEXT("Field %s was not found."), *FieldName);
	}
	return MakeShared<FJsonValueNull>();
}

int32 FJsonObject::GetIntegerField(const FString& FieldName) const
{
	int32 Result = 0;
	if (!GetField(FieldName)->TryGetNumber(Result))
	{
		UE_LOG(LogJson, Error, TEXT("Field %s is not an integer."), *FieldName);
	}
	return Result;
}

const TArray<TSharedPtr<FJsonValue>>& FJsonObject::GetArrayField(const FString& FieldName) const
{
	// The returned reference points into this object (or at a static), never into the temporary returned by GetField.
	const TSharedPtr<FJsonValue>* Field = Values.Find(FieldName);
	if (Field != nullptr && Field->IsValid() && (*Field)->Type == EJson::Array)
	{
		return (*Field)->AsArray();
	}
	return GetField(FieldName, EJson::Array)->AsArray();
}

const TSharedPtr<FJsonObject>& FJsonObject::GetObjectField(const FString& FieldName) const
{
	const TSharedPtr<FJsonValue>* Field = Values.Find(FieldName);
	if (Field != nullptr && Field->IsValid() && (*Field)->Type == EJson::Object)
	{
		return (*Field)->AsObject();
	}
	return GetField(FieldName, EJson::Object)->AsObject();
}

bool FJsonObject::TryGetStringField(const FString& FieldName, FString& OutString) const
{
	const TSharedPtr<FJsonValue>* Field = Values.Find(FieldName);
	return Field != nullptr && Field->IsValid() && (*Field)->TryGetString(OutString);
}

// ---------------------------------------------------------------------------------------------------------------------
// Parsing

namespace
{
	class FJsonParser
	{
	public:
		explicit FJsonParser(const std::wstring& InText) : Text(InText) {}

		bool Parse(TSharedPtr<FJsonValue>& OutValue)
		{
			if (!ParseValue(OutValue, 0))
			{
				return false;
			}
			SkipWhitespace();
			return Position == Text.size();
		}

	private:
		wchar_t Peek() const { return Position < Text.size() ? Text[Position] : L'\0'; }

		void SkipWhitespace()
		{
			while (Position < Text.size() && (Text[Position] == L' ' || Text[Position] == L'\t' || Text[Position] == L'\r' || Text[Position] == L'\n'))
			{
				++Position;
			}
		}

		bool ParseValue(TSharedPtr<FJsonValue>& OutValue, int Depth)
		{
			if (Depth > 256)
			{
				return false;
			}
			SkipWhitespace();
			switch (Peek())
			{
			case L'{':
				return ParseObject(OutValue, Depth);
			case L'[':
				return ParseArray(OutValue, Depth);
			case L'"':
			{
				FString String;
				if (!ParseString(String))
				{
					return false;
				}
				OutValue = MakeShared<FJsonValueString>(String);
				return true;
			}
			case L't':
				OutValue = MakeShared<FJsonValueBoolean>(true);
				return ParseLiteral(L"true");
			case L'f':
				OutValue = MakeShared<FJsonValueBoolean>(false);
				return ParseLiteral(L"false");
			case L'n':
				OutValue = MakeShared<FJsonValueNull>();
				return ParseLiteral(L"null");
			default:
				return ParseNumber(OutValue);
			}
		}

		bool ParseLiteral(const wchar_t* Literal)
		{
			const std::size_t Length = std::wcslen(Literal);
			if (Text.compare(Position, Length, Literal) != 0)
			{
				return false;
			}
			Position += Length;
			return true;
		}

		bool ParseObject(TSharedPtr<FJsonValue>& OutValue, int Depth)
		{
			++Position;
			TSharedPtr<FJsonObject> Object = MakeShared<FJsonObject>();
			SkipWhitespace();
			if (Peek() == L'}')
			{
				++Position;
				OutValue = MakeShared<FJsonValueObject>(Object);
				return true;
			}
			while (true)
			{
				SkipWhitespace();
				FString Key;
				if (Peek() != L'"' || !ParseString(Key))
				{
					return false;
				}
				SkipWhitespace();
				if (Peek() != L':')
				{
					return false;
				}
				++Position;
				TSharedPtr<FJsonValue> Value;
				if (!ParseValue(Value, Depth + 1))
				{
					return false;
				}
				Object->Values.Add(Key, Value);
				SkipWhitespace();
				if (Peek() == L',')
				{
					++Position;
					continue;
				}
				if (Peek() == L'}')
				{
					++Position;
					break;
				}
				return false;
			}
			OutValue = MakeShared<FJsonValueObject>(Object);
			return true;
		}

		bool ParseArray(TSharedPtr<FJsonValue>& OutValue, int Depth)
		{
			++Position;
			TArray<TSharedPtr<FJsonValue>> Array;
			SkipWhitespace();
			if (Peek() == L']')
			{
				++Position;
				OutValue = MakeShared<FJsonValueArray>(Array);
				return true;
			}
			while (true)
			{
				TSharedPtr<FJsonValue> Value;
				if (!ParseValue(Value, Depth + 1))
				{
					return false;
				}
				Array.Add(Value);
				SkipWhitespace();
				if (Peek() == L',')
				{
					++Position;
					continue;
				}
				if (Peek() == L']')
				{
					++Position;
					break;
				}
				return false;
			}
			OutValue = MakeShared<FJsonValueArray>(Array);
			return true;
		}

		bool ParseHex4(uint32& OutValue)
		{
			OutValue = 0;
			for (int Digit = 0; Digit < 4; ++Digit)
			{
				const wchar_t Char = Peek();
				uint32 Nibble;
				if (Char >= L'0' && Char <= L'9') { Nibble = Char - L'0'; }
				else if (Char >= L'a' && Char <= L'f') { Nibble = Char - L'a' + 10; }
				else if (Char >= L'A' && Char <= L'F') { Nibble = Char - L'A' + 10; }
				else { return false; }
				OutValue = (OutValue << 4) | Nibble;
				++Position;
			}
			return true;
		}

		bool ParseString(FString& OutString)
		{
			++Position;
			std::wstring Result;
			while (true)
			{
				if (Position >= Text.size())
				{
					return false;
				}
				const wchar_t Char = Text[Position++];
				if (Char == L'"')
				{
					break;
				}
				if (Char != L'\\')
				{
					Result += Char;
					continue;
				}
				const wchar_t Escape = Peek();
				++Position;
				switch (Escape)
				{
				case L'"': Result += L'"'; break;
				case L'\\': Result += L'\\'; break;
				case L'/': Result += L'/'; break;
				case L'b': Result += L'\b'; break;
				case L'f': Result += L'\f'; break;
				case L'n': Result += L'\n'; break;
				case L'r': Result += L'\r'; break;
				case L't': Result += L'\t'; break;
				case L'u':
				{
					uint32 CodeUnit = 0;
					if (!ParseHex4(CodeUnit))
					{
						return false;
					}
					uint32 CodePoint = CodeUnit;
					if (CodeUnit >= 0xD800 && CodeUnit <= 0xDBFF && Text.compare(Position, 2, L"\\u") == 0)
					{
						const std::size_t Saved = Position;
						Position += 2;
						uint32 Low = 0;
						if (ParseHex4(Low) && Low >= 0xDC00 && Low <= 0xDFFF)
						{
							CodePoint = 0x10000 + ((CodeUnit - 0xD800) << 10) + (Low - 0xDC00);
						}
						else
						{
							Position = Saved;
						}
					}
					if (sizeof(wchar_t) == 2 && CodePoint >= 0x10000)
					{
						Result += static_cast<wchar_t>(0xD800 + ((CodePoint - 0x10000) >> 10));
						Result += static_cast<wchar_t>(0xDC00 + ((CodePoint - 0x10000) & 0x3FF));
					}
					else
					{
						Result += static_cast<wchar_t>(CodePoint);
					}
					break;
				}
				default:
					return false;
				}
			}
			OutString = FString(std::move(Result));
			return true;
		}

		static bool IsDigit(wchar_t Char) { return Char >= L'0' && Char <= L'9'; }

		// Skips one or more digits. Returns false if there are none.
		bool SkipDigits()
		{
			const std::size_t DigitsStart = Position;
			while (IsDigit(Peek()))
			{
				++Position;
			}
			return Position > DigitsStart;
		}

		// Like Unreal's reader, accepts only numbers that follow the JSON grammar (RFC 8259):
		// -?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?. Malformed numbers such as 01, 1., .5, +1 and 1e fail the parse.
		bool ParseNumber(TSharedPtr<FJsonValue>& OutValue)
		{
			const std::size_t Start = Position;
			if (Peek() == L'-')
			{
				++Position;
			}
			if (Peek() == L'0')
			{
				++Position;
				if (IsDigit(Peek()))
				{
					return false;
				}
			}
			else if (!SkipDigits())
			{
				return false;
			}
			if (Peek() == L'.')
			{
				++Position;
				if (!SkipDigits())
				{
					return false;
				}
			}
			if (Peek() == L'e' || Peek() == L'E')
			{
				++Position;
				if (Peek() == L'+' || Peek() == L'-')
				{
					++Position;
				}
				if (!SkipDigits())
				{
					return false;
				}
			}
			const std::wstring Number = Text.substr(Start, Position - Start);
			OutValue = MakeShared<FJsonValueNumber>(std::wcstod(Number.c_str(), nullptr));
			return true;
		}

		const std::wstring& Text;
		std::size_t Position = 0;
	};

	void WriteString(std::wstring& Out, const FString& String)
	{
		Out += L'"';
		for (const wchar_t Char : String.GetStdString())
		{
			switch (Char)
			{
			case L'"': Out += L"\\\""; break;
			case L'\\': Out += L"\\\\"; break;
			case L'\b': Out += L"\\b"; break;
			case L'\f': Out += L"\\f"; break;
			case L'\n': Out += L"\\n"; break;
			case L'\r': Out += L"\\r"; break;
			case L'\t': Out += L"\\t"; break;
			default:
				if (static_cast<uint32>(Char) < 0x20)
				{
					wchar_t Escaped[8];
					std::swprintf(Escaped, 8, L"\\u%04x", static_cast<unsigned>(Char));
					Out += Escaped;
				}
				else
				{
					Out += Char;
				}
			}
		}
		Out += L'"';
	}

	void WriteNewLine(std::wstring& Out, bool bPretty, int Indent)
	{
		if (bPretty)
		{
			Out += L'\n';
			Out.append(static_cast<std::size_t>(Indent), L'\t');
		}
	}

	void WriteValue(std::wstring& Out, const TSharedPtr<FJsonValue>& Value, bool bPretty, int Indent);

	void WriteObject(std::wstring& Out, const FJsonObject& Object, bool bPretty, int Indent)
	{
		Out += L'{';
		bool bFirst = true;
		for (const auto& Pair : Object.Values)
		{
			if (!bFirst)
			{
				Out += L',';
			}
			bFirst = false;
			WriteNewLine(Out, bPretty, Indent + 1);
			WriteString(Out, Pair.Key);
			Out += bPretty ? L": " : L":";
			WriteValue(Out, Pair.Value, bPretty, Indent + 1);
		}
		if (!bFirst)
		{
			WriteNewLine(Out, bPretty, Indent);
		}
		Out += L'}';
	}

	void WriteValue(std::wstring& Out, const TSharedPtr<FJsonValue>& Value, bool bPretty, int Indent)
	{
		if (!Value.IsValid())
		{
			Out += L"null";
			return;
		}
		switch (Value->Type)
		{
		case EJson::String:
		{
			FString String;
			Value->TryGetString(String);
			WriteString(Out, String);
			break;
		}
		case EJson::Number:
		{
			double Number = 0.0;
			Value->TryGetNumber(Number);
			wchar_t Buffer[64];
			if (std::isfinite(Number) && std::floor(Number) == Number && std::fabs(Number) < 1e15)
			{
				std::swprintf(Buffer, 64, L"%lld", static_cast<long long>(Number));
			}
			else
			{
				std::swprintf(Buffer, 64, L"%.17g", Number);
			}
			Out += Buffer;
			break;
		}
		case EJson::Boolean:
		{
			bool Bool = false;
			Value->TryGetBool(Bool);
			Out += Bool ? L"true" : L"false";
			break;
		}
		case EJson::Array:
		{
			const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
			Value->TryGetArray(Array);
			Out += L'[';
			for (int32 Index = 0; Index < Array->Num(); ++Index)
			{
				if (Index > 0)
				{
					Out += L',';
				}
				WriteNewLine(Out, bPretty, Indent + 1);
				WriteValue(Out, (*Array)[Index], bPretty, Indent + 1);
			}
			if (Array->Num() > 0)
			{
				WriteNewLine(Out, bPretty, Indent);
			}
			Out += L']';
			break;
		}
		case EJson::Object:
		{
			const TSharedPtr<FJsonObject>* Object = nullptr;
			Value->TryGetObject(Object);
			if (Object->IsValid())
			{
				WriteObject(Out, **Object, bPretty, Indent);
			}
			else
			{
				Out += L"null";
			}
			break;
		}
		default:
			Out += L"null";
			break;
		}
	}
}

bool UEShim::ParseJson(const FString& Content, TSharedPtr<FJsonValue>& OutValue)
{
	FJsonParser Parser(Content.GetStdString());
	TSharedPtr<FJsonValue> Value;
	if (!Parser.Parse(Value))
	{
		return false;
	}
	OutValue = Value;
	return true;
}

FString UEShim::WriteJson(const FJsonObject& Object, bool bPretty)
{
	std::wstring Out;
	WriteObject(Out, Object, bPretty, 0);
	return FString(std::move(Out));
}

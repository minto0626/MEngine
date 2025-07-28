#pragma once
#include <string>
#include <string_view>

namespace StringUtility
{
	std::wstring ToWideString(std::string_view utf8Str);
	std::string ToUTF8String(std::wstring_view wideStr);
	std::wstring ToWideStringFromU8(std::u8string_view u8Str);
	std::string ToUpper(std::string_view utf8Str);
	std::string ToLower(std::string_view utf8Str);
};
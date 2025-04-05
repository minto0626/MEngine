#include "StringUtility.h"
#include <windows.h>

std::wstring StringUtility::ToWideString(std::string_view utf8Str)
{
    if (utf8Str.empty())
    {
        return std::wstring();
    }

    int size = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.size()), nullptr, 0);
    if (size == 0)
    {
        return std::wstring();
    }

    std::wstring result(size, L'\0');
    int converted = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.size()), result.data(), size);
    if (converted == 0)
    {
        return std::wstring();
    }

    return result;
}

std::string StringUtility::ToUTF8String(std::wstring_view wideStr)
{
    if (wideStr.empty())
    {
        return std::string();
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), static_cast<int>(wideStr.size()), nullptr, 0, nullptr, nullptr);
    if (size == 0)
    {
        return std::string();
    }

    std::string result(size, '\0');
    int converted = WideCharToMultiByte(CP_UTF8, 0, wideStr.data(), static_cast<int>(wideStr.size()), result.data(), size, nullptr, nullptr);
    if (converted == 0)
    {
        return std::string();
    }

    return result;
}

std::wstring StringUtility::ToWideStringFromU8(std::u8string_view u8Str)
{
    return ToWideString(std::string_view(reinterpret_cast<const char*>(u8Str.data()), u8Str.size()));
}

std::string StringUtility::ToUpper(std::string_view utf8Str)
{
    std::string result(utf8Str);
    for (char& c : result)
    {
        if (c >= 'a' && c <= 'z')
        {
            c = static_cast<char>(c - 'a' + 'A');
        }
    }
    return result;
}

std::string StringUtility::ToLower(std::string_view utf8Str)
{
    std::string result(utf8Str);
    for (char& c : result)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return result;
}
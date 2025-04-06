#include "Debug.h"
#pragma once

template<typename ...Args>
void Debug::LogFormat(const wchar_t* format, Args... args)
{
#if _DEBUG
	std::wstring message = std::vformat(format, std::make_wformat_args(args...));
	Log(message);
#endif
}

template<typename ...Args>
void Debug::LogFormat(const char* format, Args ...args)
{
#if _DEBUG
	std::string message = std::vformat(format, std::make_format_args(args...));
	Log(message);
#endif
}

template<typename ...Args>
void Debug::LogErrorFormat(const wchar_t* format, Args... args)
{
#if _DEBUG
	std::wstring message = std::vformat(format, std::make_wformat_args(args...));
	LogError(message);
#endif
}

template<typename ...Args>
void Debug::LogErrorFormat(const char* format, Args ...args)
{
#if _DEBUG
	std::string message = std::vformat(format, std::make_format_args(args...));
	LogError(message);
#endif
}

template<typename ...Args>
void Debug::LogWarningFormat(const wchar_t* format, Args ...args)
{
#if _DEBUG
	std::wstring message = std::vformat(format, std::make_wformat_args(args...));
	LogWarning(message);
#endif
}

template<typename ...Args>
void Debug::LogWarningFormat(const char* format, Args ...args)
{
#if _DEBUG
	std::string message = std::vformat(format, std::make_format_args(args...));
	LogWarning(message);
#endif
}

template<typename ...Args>
void Debug::LogAssertFormat(bool condition, const wchar_t* format, Args ...args)
{
#if _DEBUG
	if (!condition)
	{
		std::wstring message = std::vformat(format, std::make_wformat_args(args...));
		LogError(message);
	}
#endif
}

template<typename ...Args>
void Debug::LogAssertFormat(bool condition, const char* format, Args ...args)
{
#if _DEBUG
	if (!condition)
	{
		std::string message = std::vformat(format, std::make_format_args(args...));
		LogError(message);
	}
#endif
}
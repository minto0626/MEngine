#include "Debug.h"
#include <windows.h>
#include "StringUtility.h"

void Debug::OutputLine(const std::wstring& message)
{
	// スレッドセーフを確保
	std::lock_guard<std::mutex> lock(s_mutex);
	std::wstring output = message + L"\n";
	::OutputDebugStringW(output.c_str());
}

void Debug::Log(const std::wstring& message)
{
#if _DEBUG
	OutputLine(message);
#endif
}

void Debug::Log(const std::string& message)
{
#if _DEBUG
	Log(StringUtility::ToWideString(message));
#endif
}

void Debug::LogError(const std::wstring& message)
{
#if _DEBUG
	OutputLine(L"[Error] " + message);
#endif
}

void Debug::LogError(const std::string& message)
{
#if _DEBUG
	LogError(StringUtility::ToWideString(message));
#endif
}

void Debug::LogWarning(const std::wstring& message)
{
#if _DEBUG
	OutputLine(L"[Warning] " + message);
#endif
}

void Debug::LogWarning(const std::string& message)
{
#if _DEBUG
	LogWarning(StringUtility::ToWideString(message));
#endif
}

void Debug::LogAssert(bool condition, const std::wstring& message)
{
#if _DEBUG
	if (!condition)
	{
		LogError(message);
	}
#endif
}

void Debug::LogAssert(bool condition, const std::string& message)
{
#if _DEBUG
	if (!condition)
	{
		LogError(message);
	}
#endif
}
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

void Debug::ShowMessageBox(const std::wstring& message, const std::wstring& title, unsigned int type)
{
	MessageBoxW(nullptr, message.c_str(), title.c_str(), type);
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

void Debug::ShowMessageBox(const std::wstring& message, const std::wstring& title)
{
#if _DEBUG
	ShowMessageBox(message, title, MB_OK | MB_ICONINFORMATION);
#endif
}

void Debug::ShowMessageBox(const std::string& message, const std::string& title)
{
#if _DEBUG
	std::wstring _message = StringUtility::ToWideString(message);
	std::wstring _title = StringUtility::ToWideString(title);
	ShowMessageBox(_message, _title);
#endif
}

void Debug::ShowErrorMessageBox(const std::wstring& message, const std::wstring& title)
{
#if _DEBUG
	ShowMessageBox(message, title, MB_OK | MB_ICONERROR);
#endif
}

void Debug::ShowErrorMessageBox(const std::string& message, const std::string& title)
{
#if _DEBUG
	std::wstring _message = StringUtility::ToWideString(message);
	std::wstring _title = StringUtility::ToWideString(title);
	ShowErrorMessageBox(_message, _title);
#endif
}

void Debug::ShowWarningMessageBox(const std::wstring& message, const std::wstring& title)
{
#if _DEBUG
	ShowMessageBox(message, title, MB_OK | MB_ICONWARNING);
#endif
}

void Debug::ShowWarningMessageBox(const std::string& message, const std::string& title)
{
#if _DEBUG
	std::wstring _message = StringUtility::ToWideString(message);
	std::wstring _title = StringUtility::ToWideString(title);
	ShowWarningMessageBox(_message, _title);
#endif
}
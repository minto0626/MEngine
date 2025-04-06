#pragma once
#include <string>
#include <format>
#include <mutex>

class Debug
{
private:
	// スレッドセーフの為のmutex
	static std::mutex s_mutex;

	static void OutputLine(const std::wstring& message);
	static void ShowMessageBox(const std::wstring& message, const std::wstring& title, unsigned int type);

public:
	static void Log(const std::wstring& message);
	static void Log(const std::string& message);
	static void LogError(const std::wstring& message);
	static void LogError(const std::string& message);
	static void LogWarning(const std::wstring& message);
	static void LogWarning(const std::string& message);
	static void LogAssert(bool condition, const std::wstring& message);
	static void LogAssert(bool condition, const std::string& message);

	template<typename... Args>
	static void LogFormat(const wchar_t* format, Args... args);
	template<typename... Args>
	static void LogFormat(const char* format, Args... args);
	template<typename... Args>
	static void LogErrorFormat(const wchar_t* format, Args... args);
	template<typename... Args>
	static void LogErrorFormat(const char* format, Args... args);
	template<typename... Args>
	static void LogWarningFormat(const wchar_t* format, Args... args);
	template<typename... Args>
	static void LogWarningFormat(const char* format, Args... args);
	template<typename... Args>
	static void LogAssertFormat(bool condition, const wchar_t* format, Args... args);
	template<typename... Args>
	static void LogAssertFormat(bool condition, const char* format, Args... args);

public:
	static void ShowMessageBox(const std::wstring& message, const std::wstring& title);
	static void ShowMessageBox(const std::string& message, const std::string& title);
	static void ShowErrorMessageBox(const std::wstring& message, const std::wstring& title);
	static void ShowErrorMessageBox(const std::string& message, const std::string& title);
	static void ShowWarningMessageBox(const std::wstring& message, const std::wstring& title);
	static void ShowWarningMessageBox(const std::string& message, const std::string& title);

};

#include "Debug.inl"

inline std::mutex Debug::s_mutex;
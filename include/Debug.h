#pragma once
#include <string>
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

public:
	static void ShowMessageBox(const std::wstring& message, const std::wstring& title);
	static void ShowMessageBox(const std::string& message, const std::string& title);
	static void ShowErrorMessageBox(const std::wstring& message, const std::wstring& title);
	static void ShowErrorMessageBox(const std::string& message, const std::string& title);
	static void ShowWarningMessageBox(const std::wstring& message, const std::wstring& title);
	static void ShowWarningMessageBox(const std::string& message, const std::string& title);

};

inline std::mutex Debug::s_mutex;
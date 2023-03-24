#pragma once

#include <string>
#include <cstdarg>

class Logger
{
public:
	Logger(const char* prefix)
	{
		printPrefix = prefix;

		FILE* logFile = GetLogFile();
		if (logFile == nullptr)
		{
			fopen_s(&logFile, "hook_log.txt", "w");
			GetLogFile(logFile);
		}
	}

	void Log(std::string msg, ...)
	{
		va_list args;
		va_start(args, msg);
		vprintf(std::string(printPrefix + " > " + msg + "\n").c_str(), args);
		if (GetLogFile() != nullptr)
		{
			vfprintf(GetLogFile(), std::string(printPrefix + " > " + msg + "\n").c_str(), args);
			fflush(GetLogFile());
		}
		va_end(args);
	}

	void LogW(std::wstring msg, ...)
	{
		va_list args;
		va_start(args, msg);
		vwprintf(std::wstring(m_wprintPrefix + L" > " + msg + L"\n").c_str(), args);
		if (GetLogFile() != nullptr)
		{
			vfwprintf(GetLogFile(), std::wstring(m_wprintPrefix + L" > " + msg + L"\n").c_str(), args);
			fflush(GetLogFile());
		}
		va_end(args);
	}

private:
	std::string printPrefix = "";
	std::wstring m_wprintPrefix = L"";

	static FILE* GetLogFile(FILE* newLogFile = nullptr)
	{
		static FILE* logFile = nullptr;
		if (newLogFile != nullptr)
		{
			logFile = newLogFile;
		}
		return logFile;
	}
};
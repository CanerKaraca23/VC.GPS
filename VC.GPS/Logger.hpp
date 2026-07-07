#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <windows.h>

class Logger {
public:
    static void Init(const std::string& logFilePath, bool enable);
    static void Log(const char* format, ...);

private:
    static std::string logPath;
    static bool enabled;
    static std::mutex mtx;
};

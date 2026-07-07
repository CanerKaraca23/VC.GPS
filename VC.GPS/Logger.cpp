#include "Logger.hpp"
#include <cstdarg>
#include <cstdio>
#include <ctime>

std::string Logger::logPath = "";
bool Logger::enabled = false;
std::mutex Logger::mtx;

void Logger::Init(const std::string& logFilePath, bool enable) {
    logPath = logFilePath;
    enabled = enable;

    if (enabled) {
        std::ofstream file(logPath, std::ios::out | std::ios::trunc);
        if (file.is_open()) {
            file << "--- VC.GPS Log Started ---\n";
            file.close();
        }
    }
}

void Logger::Log(const char* format, ...) {
    if (!enabled || logPath.empty()) return;

    std::lock_guard<std::mutex> lock(mtx);

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    time_t rawtime;
    struct tm timeinfo;
    char timeBuffer[80];
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(timeBuffer, sizeof(timeBuffer), "[%H:%M:%S] ", &timeinfo);

    std::ofstream file(logPath, std::ios::out | std::ios::app);
    if (file.is_open()) {
        file << timeBuffer << buffer << "\n";
        file.close();
    }
}

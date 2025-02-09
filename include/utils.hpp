#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <iostream>
#include <iomanip>

// Add this before the namespace declaration
#define LOG(message, level) Logger::log(message, __func__, level)

namespace pinrex {

// Color definitions for terminal output
namespace Color {
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
}

// Log levels for different types of messages
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// Progress bar for showing operation progress
class ProgressBar {
public:
    explicit ProgressBar(size_t total, size_t width = 70);
    void update(size_t current);
    void finish();

private:
    size_t total_;
    size_t width_;
    size_t last_printed_percent_;
};

// Logger class for handling log messages
class Logger {
public:
    static void init(const std::string& logFile = "");
    static void log(const std::string& message, 
                   const std::string& function_name,
                   LogLevel level = LogLevel::INFO);
    static void setLogFile(const std::string& path);
    static void setVerbose(bool verbose);

private:
    static std::ofstream logFile_;
    static bool initialized_;
    static bool verbose_;
    static std::string getTimestamp();
    static std::string getLevelString(LogLevel level);
    static std::string getColoredLevel(LogLevel level);
};

} // namespace pinrex 
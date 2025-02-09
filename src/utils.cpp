#include "utils.hpp"
#include <iomanip>  // for put_time, setfill, setw
#include <sstream>  // for stringstream

namespace pinrex {

// Static member initialization
std::ofstream Logger::logFile_;
bool Logger::initialized_ = false;
bool Logger::verbose_ = false;  // Default to non-verbose

// ProgressBar implementation
ProgressBar::ProgressBar(size_t total, size_t width) 
    : total_(total), width_(width), last_printed_percent_(0) {
    // Print initial empty progress bar
    std::cout << "[" << std::string(width_, ' ') << "] 0%\r";
    std::cout.flush();
}

void ProgressBar::update(size_t current) {
    if (current > total_) current = total_;
    
    float progress = static_cast<float>(current) / total_;
    size_t current_percent = static_cast<size_t>(progress * 100);
    
    // Only update if percentage changed (reduces flickering)
    if (current_percent != last_printed_percent_) {
        last_printed_percent_ = current_percent;
        
        size_t pos = static_cast<size_t>(width_ * progress);
        std::cout << "[";
        for (size_t i = 0; i < width_; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << current_percent << "%\r";
        std::cout.flush();
    }
}

void ProgressBar::finish() {
    update(total_);
    std::cout << std::endl;
}

// Logger implementation
void Logger::init(const std::string& logFile) {
    if (!initialized_) {
        if (!logFile.empty()) {
            logFile_.open(logFile, std::ios::app);
        }
        initialized_ = true;
    }
}

void Logger::setLogFile(const std::string& path) {
    if (logFile_.is_open()) {
        logFile_.close();
    }
    logFile_.open(path, std::ios::app);
}

void Logger::setVerbose(bool verbose) {
    verbose_ = verbose;
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        default:               return "UNKNOWN";
    }
}

std::string Logger::getColoredLevel(LogLevel level) {
    std::string levelStr = getLevelString(level);
    switch (level) {
        case LogLevel::DEBUG:   return Color::BLUE + levelStr + Color::RESET;
        case LogLevel::INFO:    return Color::GREEN + levelStr + Color::RESET;
        case LogLevel::WARNING: return Color::YELLOW + levelStr + Color::RESET;
        case LogLevel::ERROR:   return Color::RED + levelStr + Color::RESET;
        default:               return levelStr;
    }
}

void Logger::log(const std::string& message, const std::string& function_name, LogLevel level) {
    // Only output to console if verbose mode is enabled
    if (verbose_) {
        std::string timestamp = getTimestamp();
        std::string levelStr = getColoredLevel(level);
        std::cout << timestamp << " [" << levelStr << "] [" 
                 << Color::CYAN << function_name << Color::RESET << "] " 
                 << message << std::endl;
    }
    
    // Always write to log file if it's open
    if (logFile_.is_open()) {
        std::string timestamp = getTimestamp();
        logFile_ << timestamp << " [" << getLevelString(level) << "] [" 
                << function_name << "] " 
                << message << std::endl;
    }
}

} // namespace pinrex 
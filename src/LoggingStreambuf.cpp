#include "LoggingStreambuf.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>

LoggingStreambuf::LoggingStreambuf()
    : m_cout_buf(std::cout.rdbuf()), m_log_file(nullptr) {
}

LoggingStreambuf::~LoggingStreambuf() {
    stopLogging();  // Ensure file is closed
    sync();         // Flush any remaining data just incase
}

void LoggingStreambuf::startLogging(const std::string& commandName) {
    // timestamp and date of command
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto local_t = std::localtime(&time_t);
    
    // Create directories (does nothing if they already exist)
    std::stringstream dirpath;    
    dirpath << "logs/" << std::put_time(local_t, "%Y%m%d");
    std::filesystem::create_directories(dirpath.str());   

    std::stringstream filepath;    // Create full filepath
    filepath << dirpath.str() << "/" << commandName << "_" << std::put_time(local_t, "%H%M%S") << ".txt";
    
    // Start logging to this file
    stopLogging();  // Close any existing file
    m_log_file = std::make_unique<std::ofstream>(filepath.str(), std::ios::app);
}

void LoggingStreambuf::stopLogging() {
    if (m_log_file && m_log_file->is_open()) {
        m_log_file->close();
    }
    m_log_file.reset();
}

int LoggingStreambuf::overflow(int c) {
    if (c == EOF) {
        return EOF;
    }
    if (m_cout_buf->sputc(c) == EOF) {    // Write to console
        return EOF;
    }
    if (m_log_file && m_log_file->is_open()) {    // Write to file
        m_log_file->put(c);
        m_log_file->flush();  // Immediate flush for real-time file output
    }
    return c;
}

int LoggingStreambuf::sync() {
    if (m_cout_buf->pubsync() == -1) {    // Sync console buffer
        return -1;
    }
    if (m_log_file && m_log_file->is_open()) {    // Sync file stream
        m_log_file->flush();
    }
    return 0;
}
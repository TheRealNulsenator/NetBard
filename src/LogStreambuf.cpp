#include "LogStreambuf.h"
#include <iostream>
#include <iomanip>
#include <filesystem>

LogStreambuf::LogStreambuf()
    : m_cout_buf(std::cout.rdbuf()), m_log_file(nullptr) {
        // Create directories (does nothing if they already exist)
        auto local_t = timestamp(); 
        dirPath << "logs/" << std::put_time(&local_t, "%Y%m%d");
        std::filesystem::create_directories(dirPath.str());   
    }

LogStreambuf::~LogStreambuf() {
    stopLogging();  // Ensure file is closed
    sync();         // Flush any remaining data just incase
}

void LogStreambuf::startLogging(const std::string& fileName) {

    auto local_t = timestamp();

    std::stringstream filepath;    // Create full filepath
    filepath << dirPath.str() << "/" << fileName << "_" << std::put_time(&local_t, "%H%M%S") << ".txt";

    m_log_file = std::make_unique<std::ofstream>(filepath.str(), std::ios::app);
    std::cout.rdbuf(this);    // Start logging to the specified file
}

void LogStreambuf::stopLogging() {
    if (m_log_file && m_log_file->is_open()) {
        m_log_file->close();
    }
    m_log_file.reset();
}

int LogStreambuf::overflow(int c) {
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

int LogStreambuf::sync() {
    if (m_cout_buf->pubsync() == -1) {    // Sync console buffer
        return -1;
    }
    if (m_log_file && m_log_file->is_open()) {    // Sync file stream
        m_log_file->flush();
    }
    return 0;
}

tm LogStreambuf::timestamp(){ 
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    return *std::localtime(&time_t);
}
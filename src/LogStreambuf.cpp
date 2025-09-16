#include "LogStreambuf.h"
#include <iostream>
#include <iomanip>
#include <filesystem>

std::streambuf* LogStreambuf::s_cout_original_buf = nullptr;

LogStreambuf::LogStreambuf(std::string title){
        m_file_title = title;
        m_log_file = nullptr;
        if(!s_cout_original_buf){ //private static pointer of original std::cout streambuf
            s_cout_original_buf = std::cout.rdbuf();
        }
        // Create directories (does nothing if they already exist)
        auto local_t = timestamp(); 
        m_directory << "logs/" << std::put_time(&local_t, "%Y%m%d");
        std::filesystem::create_directories(m_directory.str());   
    }

LogStreambuf::~LogStreambuf() {
    stopLogging();  // Ensure file is closed
    sync();         // Flush any remaining data just incase
}

void LogStreambuf::startLogging(const std::string& details) {
    stopLogging();  // Ensure file is closed
    auto local_t = timestamp();
    std::stringstream filepath;    // Create full filepath
    filepath    << m_directory.str() << "/" << m_file_title     //title for unique command identifier
                << "_" << sanitize_for_windows_path(details)    //details of this call
                << "_" << std::put_time(&local_t, "%H%M%S") << ".txt"; //timestamp
    m_log_file = std::make_unique<std::ofstream>(filepath.str(), std::ios::app);
    std::cout.rdbuf(this);    // Start logging to the specified file
}

void LogStreambuf::stopLogging() {
    if (m_log_file && m_log_file->is_open()) {
        m_log_file->close();
    }
    m_log_file.reset();
    if (std::cout.rdbuf() == this){
        std::cout.rdbuf(s_cout_original_buf);
    }
}

int LogStreambuf::overflow(int c) {
    if (c == EOF) {
        return EOF;
    }
    if (s_cout_original_buf->sputc(c) == EOF) {    // Write to console
        return EOF;
    }
    if (m_log_file && m_log_file->is_open()) {    // Write to file
        m_log_file->put(c);
        m_log_file->flush();  // Immediate flush for real-time file output
    }
    return c;
}

int LogStreambuf::sync() {
    if (s_cout_original_buf->pubsync() == -1) {    // Sync console buffer
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


std::string LogStreambuf::sanitize_for_windows_path(const std::string& filename) {
    std::string result = filename;
    
    // Characters not allowed in Windows filenames
    const std::string invalidChars = "<>:\"/\\|?*";
    
    for (char& c : result) {    // Replace each invalid character
        if (invalidChars.find(c) != std::string::npos) {
            c = '-';
        }
        // Also replace control characters (ASCII 0-31)
        if (c >= 0 && c <= 31) {
            c = '-';
        }
    }
    
    return result;
}
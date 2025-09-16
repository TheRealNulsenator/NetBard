#ifndef LOGGING_STREAMBUF_H
#define LOGGING_STREAMBUF_H

#include <streambuf>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>
#include <chrono>

class LogStreambuf : public std::streambuf {
public:
    LogStreambuf(std::string title);
    virtual ~LogStreambuf();
    
    void startLogging(const std::string& details);
    void stopLogging();
    
protected:
    // Override streambuf methods to write to both destinations
    virtual int overflow(int c) override;
    virtual int sync() override;
    
private:
    std::stringstream m_directory;
    std::string m_file_title;
    static std::streambuf* s_cout_original_buf;                     // Original cout buffer
    std::unique_ptr<std::ofstream> m_log_file;      // Log file stream (owned by LoggingStreambuf)
    tm timestamp();
    std::string sanitize_for_windows_path(const std::string& filename);
};

#endif // LOGGING_STREAMBUF_H
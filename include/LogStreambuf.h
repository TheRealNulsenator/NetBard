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
    LogStreambuf();
    virtual ~LogStreambuf();
    
    void startLogging(const std::string& fileName);
    void stopLogging();
    
protected:
    // Override streambuf methods to write to both destinations
    virtual int overflow(int c) override;
    virtual int sync() override;
    
private:
    std::stringstream dirPath;
    std::streambuf* m_cout_buf;                     // Original cout buffer
    std::unique_ptr<std::ofstream> m_log_file;      // Log file stream (owned by LoggingStreambuf)
    tm timestamp();
};

#endif // LOGGING_STREAMBUF_H
#ifndef LOGGING_STREAMBUF_H
#define LOGGING_STREAMBUF_H

#include <streambuf>
#include <fstream>
#include <memory>
#include <string>

class LoggingStreambuf : public std::streambuf {
public:
    LoggingStreambuf();
    virtual ~LoggingStreambuf();
    
    // Start/stop logging to a file
    void startLogging(const std::string& commandName);
    void stopLogging();
    
protected:
    // Override streambuf methods to write to both destinations
    virtual int overflow(int c) override;
    virtual int sync() override;
    
private:
    std::streambuf* m_cout_buf;                     // Original cout buffer
    std::unique_ptr<std::ofstream> m_log_file;      // Log file stream (owned by LoggingStreambuf)
};

#endif // LOGGING_STREAMBUF_H
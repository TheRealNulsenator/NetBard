#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>
#include <fstream>
#include <streambuf>

class CommandDispatcher {
public:
    // Static interface - no instances needed
    static void initialize();    // Initialize built-in commands
    static bool processCommand(const std::string& command);    // Process a command and return false if program should exit
    
    // Register a command handler (all handlers take vector of arguments)
    using CommandHandler = std::function<bool(const std::vector<std::string>&)>;
    static void registerCommand(const std::string& name, CommandHandler handler, const std::string& tip ="");

private:
    // Nested class for tee functionality - duplicates output to cout and log file
    class TeeStreambuf : public std::streambuf {
    public:
        TeeStreambuf(std::streambuf* cout_buf, std::ofstream* log_file);
        virtual ~TeeStreambuf();
        
    protected:
        // Override streambuf methods to write to both destinations
        virtual int overflow(int c) override;
        virtual int sync() override;
        
    private:
        std::streambuf* m_cout_buf;      // Original cout buffer
        std::ofstream* m_log_file;       // Log file stream
    };
    
    // Static data members
    static std::unordered_map<std::string, CommandHandler> s_commands;     // Map of command names to handler functions
    static std::unordered_map<std::string, std::string> s_tips;            // Map of command names to tips
    static bool s_initialized;
    
    // Logging members
    static std::unique_ptr<std::ofstream> s_logFile;           // Current log file stream
    static std::unique_ptr<TeeStreambuf> s_teeStreambuf;       // Tee streambuf instance
    static std::streambuf* s_originalCoutBuffer;               // Original cout buffer to restore
    
    // Static helper functions
    static void initializeBuiltInCommands();
    static bool handleHelp(const std::vector<std::string>& arguments);
    static std::vector<std::string> splitCommand(const std::string& command);
    
    // Logging functions
    static void startLogging(const std::string& commandName);  // Start logging for a command
    static void stopLogging();                                 // Stop logging and restore cout

    // Prevent instantiation
    CommandDispatcher() = delete;
    ~CommandDispatcher() = delete;
    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;
};

#endif // COMMAND_DISPATCHER_H
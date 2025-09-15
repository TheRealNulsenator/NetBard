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
    
    // command handler delegate (all handlers take vector of arguments)
    using CommandHandler = std::function<bool(const std::vector<std::string>&)>;
    static void registerCommand(const std::string& name, CommandHandler handler, const std::string& tip ="");

private:

    // Nested class for tee functionality - duplicates output to cout and log file
    class TeeStreambuf : public std::streambuf { //trick here is that we inherit streambuf, so cout will accept this
        public:
            TeeStreambuf();
            virtual ~TeeStreambuf();
            
            // Start/stop logging to a file
            void startLogging(const std::string& filepath);
            void stopLogging();
            
        protected:
            // Override streambuf methods to write to both destinations
            virtual int overflow(int c) override;
            virtual int sync() override;
            
        private:
            std::streambuf* m_cout_buf;      // Original cout buffer
            std::unique_ptr<std::ofstream> m_log_file;  // Log file stream (owned by TeeStreambuf)
    };
    
    // Static data members
    static std::unordered_map<std::string, CommandHandler> s_commands;     // Map of command names to handler functions
    static std::unordered_map<std::string, std::string> s_tips;            // Map of command names to tips
    static bool s_initialized;
    
    // Logging members
    static std::unique_ptr<TeeStreambuf> s_teeStreambuf;    // Tee streambuf instance
    
    // Static helper functions
    static void initializeBuiltInCommands();
    static bool handleHelp(const std::vector<std::string>& arguments);
    static std::vector<std::string> splitCommand(const std::string& command);
    
    // Logging functions
    static void startLogging(const std::string& commandName);  // Start logging for a command
    static void stopLogging();                                 // Stop logging and restore cout

    // Prevent instantiation because this is a static class
    CommandDispatcher() = delete;
    ~CommandDispatcher() = delete;
    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;
};

#endif // COMMAND_DISPATCHER_H
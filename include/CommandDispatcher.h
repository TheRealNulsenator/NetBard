#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>
#include "LoggingStreambuf.h"

class CommandDispatcher {
public:
    // Static interface - no instances needed
    static void initialize();    // Initialize built-in commands
    static bool processCommand(const std::string& command);    // Process a command and return false if program should exit
    
    // command handler signature (all handlers take vector of string args and return a bool)
    using CommandHandler = std::function<bool(const std::vector<std::string>&)>;
    static void registerCommand(const std::string& name, CommandHandler handler, const std::string& tip ="");

private:
    // Static data members
    static std::unordered_map<std::string, CommandHandler> s_commands;     // Map of command names to handler functions
    static std::unordered_map<std::string, std::string> s_tips;            // Map of command names to tips
    static bool s_initialized;
    
    // Logging members
    static std::unique_ptr<LoggingStreambuf> s_loggingStreambuf;    // Logging streambuf instance
    
    // Static helper functions
    static void initializeBuiltInCommands();
    static bool handleHelp(const std::vector<std::string>& arguments);
    static std::vector<std::string> splitCommand(const std::string& command);

    // Prevent instantiation because this is a static class
    CommandDispatcher() = delete;
    ~CommandDispatcher() = delete;
    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;
};

#endif // COMMAND_DISPATCHER_H
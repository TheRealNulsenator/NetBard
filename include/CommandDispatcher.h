#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

class CommandDispatcher {
public:
    CommandDispatcher();
    
    bool processCommand(const std::string& command);    // Process a command and return false if program should exit
    
    // Register a command handler (all handlers take vector of arguments)
    using CommandHandler = std::function<bool(const std::vector<std::string>&)>;
    void registerCommand(const std::string& name, CommandHandler handler, const std::string& tip ="");

    // Static method to get the single instance
    static CommandDispatcher& getInstance() {
        static CommandDispatcher instance; // Lazily initialized, thread-safe since C++11
        return instance;
    }
    
private:

    std::unordered_map<std::string, CommandHandler> m_commands;     // Map of command names to handler functions
    std::unordered_map<std::string, std::string> m_tips;            // Map of command names to tips
    
    void initializeBuiltInCommands();
    bool handleHelp(const std::vector<std::string>& arguments);
    std::vector<std::string> splitCommand(const std::string& command);

    // Delete copy constructor and assignment operator to prevent copying
    CommandDispatcher(const CommandDispatcher&) = delete;
    CommandDispatcher& operator=(const CommandDispatcher&) = delete;
};

#endif // COMMAND_DISPATCHER_H
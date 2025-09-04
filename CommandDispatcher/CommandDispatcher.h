#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <string>
#include <unordered_map>
#include <functional>

class CommandDispatcher {
public:
    CommandDispatcher();
    
    // Process a command and return false if program should exit
    bool processCommand(const std::string& command);
    
    // Register custom commands (for future extensibility)
    void registerCommand(const std::string& name, std::function<bool()> handler);
    
private:
    // Built-in command handlers
    bool handleHelp();
    bool handleQuit();
    
    // Map of command names to handler functions
    std::unordered_map<std::string, std::function<bool()>> m_commands;
    
    void initializeCommands();
};

#endif // COMMAND_DISPATCHER_H
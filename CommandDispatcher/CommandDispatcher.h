#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

class CommandDispatcher {
public:
    CommandDispatcher();
    
    // Process a command and return false if program should exit
    bool processCommand(const std::string& command);
    
    // Register a command handler (all handlers take vector of arguments)
    using CommandHandler = std::function<bool(const std::vector<std::string>&)>;
    void registerCommand(const std::string& name, CommandHandler handler, const std::string& tip ="");
    
private:

    std::unordered_map<std::string, CommandHandler> m_commands;     // Map of command names to handler functions
    std::unordered_map<std::string, std::string> m_tips;            // Map of command names to tips
    
    void initializeBuiltInCommands();
    bool handleHelp(const std::vector<std::string>& arguments);
    std::vector<std::string> splitCommand(const std::string& command);
};

#endif // COMMAND_DISPATCHER_H
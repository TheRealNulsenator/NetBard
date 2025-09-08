#include "CommandDispatcher.h"
#include <iostream>
#include <sstream>

// Static member definitions
std::unordered_map<std::string, CommandDispatcher::CommandHandler> CommandDispatcher::s_commands;
std::unordered_map<std::string, std::string> CommandDispatcher::s_tips;
bool CommandDispatcher::s_initialized = false;

void CommandDispatcher::initialize() {
    if (!s_initialized) {
        initializeBuiltInCommands();
        s_initialized = true;
    }
}

void CommandDispatcher::initializeBuiltInCommands() {
    // Register built-in commands that are always available
    registerCommand("help", [](const std::vector<std::string>& args) {
        return handleHelp(args);
    });
    
    registerCommand("quit", [](const std::vector<std::string>& args) {
        return false;  // Stop running
    });
    
    registerCommand("exit", [](const std::vector<std::string>& args) {
        return false;  // Stop running
    });
}

bool CommandDispatcher::handleHelp(const std::vector<std::string>& arguments) {
    std::cout << "\nAvailable commands:" << std::endl;
    std::cout << "  help    -   Show this message" << std::endl;
    std::cout << "  quit    -   Exit the program" << std::endl;
    std::cout << "  exit    -   Exit the program" << std::endl;
    
    // Show registered commands (excluding built-ins we just listed)
    for (const auto& commandPair : s_commands) {
        const std::string& commandName = commandPair.first;
        const std::string& tip = s_tips[commandName];
        if (commandName != "help" && commandName != "quit" && commandName != "exit") {
            std::cout << "  " << commandName << "   -   " << tip << std::endl;
        }
    }
    
    return true;  // Continue running
}

bool CommandDispatcher::processCommand(const std::string& command) {
    std::vector<std::string> commandParts = splitCommand(command);
    
    if (commandParts.empty()) { // Empty command, continue running
        return true;  
    }
    
    std::string commandName = commandParts[0];

    auto commandHandlerIterator = s_commands.find(commandName);
    if (commandHandlerIterator != s_commands.end()) {   //we found a matching command
        commandParts.erase(commandParts.begin());   // Remove command name from arguments before passing to handler
        auto commandHandler = commandHandlerIterator->second;   //get pointer to the command function itself
        bool shouldContinueRunning = commandHandler(commandParts);  //execute command with arguments
        return shouldContinueRunning;   //return results of command to main control loop
    }
    
    std::cout << "Unknown command: " << command << std::endl;
    std::cout << "Type 'help' for available commands" << std::endl;
    return true;  // Continue running
}

void CommandDispatcher::registerCommand(const std::string& name, CommandHandler handler, const std::string& tip) {
    s_commands[name] = handler;
    s_tips[name] = tip;
}

std::vector<std::string> CommandDispatcher::splitCommand(const std::string& command) {
    std::vector<std::string> parts;
    std::stringstream stringStream(command);
    std::string part;
    
    while (stringStream >> part) {
        parts.push_back(part);
    }
    
    return parts;
}
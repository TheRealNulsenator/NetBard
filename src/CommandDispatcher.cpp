#include "CommandDispatcher.h"
#include <iostream>
#include <sstream>

CommandDispatcher::CommandDispatcher() {
    initializeBuiltInCommands();
}

void CommandDispatcher::initializeBuiltInCommands() {
    // Register built-in commands that are always available
    registerCommand("help", [this](const std::vector<std::string>& args) {
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
    std::cout << "  help  - Show this message" << std::endl;
    std::cout << "  quit  - Exit the program" << std::endl;
    std::cout << "  exit  - Exit the program" << std::endl;
    
    // Show registered commands (excluding built-ins we just listed)
    for (const auto& commandPair : m_commands) {
        const std::string& commandName = commandPair.first;
        const std::string& tip = m_tips[commandName];
        if (commandName != "help" && commandName != "quit" && commandName != "exit") {
            std::cout << "  " << commandName << " - " << tip << std::endl;
        }
    }
    
    return true;  // Continue running
}

bool CommandDispatcher::processCommand(const std::string& command) {
    std::vector<std::string> commandParts = splitCommand(command);
    
    if (commandParts.empty()) {
        return true;  // Empty command, continue running
    }
    
    std::string commandName = commandParts[0];
    
    auto commandHandlerIterator = m_commands.find(commandName);
    
    if (commandHandlerIterator != m_commands.end()) {
        // Remove command name from arguments before passing to handler
        commandParts.erase(commandParts.begin());
        
        auto commandHandler = commandHandlerIterator->second;
        bool shouldContinueRunning = commandHandler(commandParts);
        return shouldContinueRunning;
    }
    
    std::cout << "Unknown command: " << command << std::endl;
    std::cout << "Type 'help' for available commands" << std::endl;
    return true;  // Continue running
}

void CommandDispatcher::registerCommand(const std::string& name, CommandHandler handler, const std::string& tip) {
    m_commands[name] = handler;
    m_tips[name] = tip;
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
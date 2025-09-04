#include "CommandDispatcher.h"
#include <iostream>

CommandDispatcher::CommandDispatcher() {
    initializeCommands();
}

void CommandDispatcher::initializeCommands() {     // Register built-in commands
    m_commands["help"] = [this]() { return handleHelp(); };
    m_commands["quit"] = [this]() { return handleQuit(); };
    m_commands["exit"] = [this]() { return handleQuit(); };
}

bool CommandDispatcher::processCommand(const std::string& command) {
    auto it = m_commands.find(command);
    
    if (it != m_commands.end()) {
        return it->second();
    }
    
    std::cout << "Unknown command: " << command << std::endl;
    std::cout << "Type 'help' for available commands" << std::endl;
    return true;  // Continue running
}

void CommandDispatcher::registerCommand(const std::string& name, std::function<bool()> handler) {
    m_commands[name] = handler;
}

bool CommandDispatcher::handleHelp() {
    std::cout << "\nAvailable commands:" << std::endl;
    std::cout << "  help  - Show this message" << std::endl;
    std::cout << "  quit  - Exit the program" << std::endl;
    std::cout << "  exit  - Exit the program" << std::endl;
    return true;  // Continue running
}

bool CommandDispatcher::handleQuit() {
    return false;  // Stop running
}
#include "CommandDispatcher.h"
#include <iostream>
#include <sstream>

// Static member definitions
std::unordered_map<std::string, CommandDispatcher::CommandHandler> CommandDispatcher::s_commands;
std::unordered_map<std::string, std::string> CommandDispatcher::s_tips;
bool CommandDispatcher::s_running = false;
//std::unique_ptr<LogStreambuf> CommandDispatcher::s_loggingStreambuf;

void CommandDispatcher::initialize() {
    if (!s_running) {
        registerCommand("help", [](const std::vector<std::string>& args) {
            return handleHelp(args);
        }, "Show this message");
        registerCommand("quit", [](const std::vector<std::string>& args) {
            s_running = false;  // Stop running
        }, "Exit the program");
        registerCommand("exit", [](const std::vector<std::string>& args) {
            s_running = false;  // Stop running
        }, "Exit the program");
        s_running = true;
    }
}

bool CommandDispatcher::handleHelp(const std::vector<std::string>& arguments) {
    std::cout << "\nAvailable commands:" << std::endl;
    for (const auto& commandPair : s_commands) { // output all registered command tips
        const std::string& commandName = commandPair.first;
        const std::string& tip = s_tips[commandName];
        std::cout << "  " << commandName << "   -   " << tip << std::endl;
        
    }
    return true;
}

void CommandDispatcher::processCommand(const std::string& command) {
    std::vector<std::string> commandParts = splitCommand(command);
    if (commandParts.empty()) { // Empty command, continue running
        return;  
    }  
    std::string commandName = commandParts[0];
    auto commandHandlerIterator = s_commands.find(commandName);
    if (commandHandlerIterator != s_commands.end()) {   //we found a matching command
        //s_loggingStreambuf->startLogging(commandName); // Start logging for this command    
        commandParts.erase(commandParts.begin());   // Remove command name from arguments before passing to handler
        auto commandHandler = commandHandlerIterator->second;   //get pointer to the command function itself
        commandHandler(commandParts);  //execute command with arguments
        //s_loggingStreambuf->stopLogging(); // Stop logging after command completes
        return;   //return results of command to main control loop
    }
    std::cout << "Unknown command: " << command << std::endl;
    std::cout << "Type 'help' for available commands" << std::endl;
    return;
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



#include "CommandDispatcher.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

// Static member definitions
std::unordered_map<std::string, CommandDispatcher::CommandHandler> CommandDispatcher::s_commands;
std::unordered_map<std::string, std::string> CommandDispatcher::s_tips;
bool CommandDispatcher::s_running = false;

void CommandDispatcher::initialize() {
    if (!s_running) { //set to running, and register built-in commands
        registerCommand("help", [](const std::vector<std::string>& args) {
            return handleHelp(args);
        }, "Show this message");
        registerCommand("quit", [](const std::vector<std::string>& args) {
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
        std::cout << "  " << commandName << "\t-\t" << tip << std::endl;

    }
    return true;
}

void CommandDispatcher::processCommand(const std::string& command) {
    std::vector<std::string> commandArgs = splitCommand(command);
    if (commandArgs.empty()) { // Empty command, continue running
        return;
    }
    std::string commandName = commandArgs[0];   //extract unique command key

    // Convert command name to lowercase for case-insensitive matching
    std::transform(commandName.begin(), commandName.end(), commandName.begin(),
                   [](unsigned char c){ return tolower(c); });

    commandArgs.erase(commandArgs.begin());     // Remove command name from arguments before passing to handler
    auto commandHandlerIterator = s_commands.find(commandName);
    if (commandHandlerIterator != s_commands.end()) {   //we found a matching command
        auto commandHandler = commandHandlerIterator->second;   //get pointer to the command function itself using commandname key
        commandHandler(commandArgs);
        return;
    }
    std::cout << "Unknown command: " << command << std::endl;
    std::cout << "Type 'help' for available commands" << std::endl;
    return;
}

void CommandDispatcher::registerCommand(const std::string& name, CommandHandler handler, const std::string& tip) {
    s_commands[name] = handler;
    s_tips[name] = tip;
}

std::vector<std::string> CommandDispatcher::splitCommand(const std::string& command) { //breaks up string into vector of seperate words
    std::vector<std::string> parts;
    std::stringstream stringStream(command);
    std::string part;

    while (stringStream >> part) {
        parts.push_back(part);
    }

    return parts;
}


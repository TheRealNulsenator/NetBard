#include "CommandDispatcher.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>

// Static member definitions
std::unordered_map<std::string, CommandDispatcher::CommandHandler> CommandDispatcher::s_commands;
std::unordered_map<std::string, std::string> CommandDispatcher::s_tips;
bool CommandDispatcher::s_initialized = false;
std::unique_ptr<CommandDispatcher::TeeStreambuf> CommandDispatcher::s_teeStreambuf;

void CommandDispatcher::initialize() {
    if (!s_initialized) {
        initializeBuiltInCommands();
        // Create a tee stream to log all CLI output
        s_teeStreambuf = std::make_unique<TeeStreambuf>();
        // Redirect cout to always use the tee streambuf
        std::cout.rdbuf(s_teeStreambuf.get());
        s_initialized = true;
    }
}

void CommandDispatcher::initializeBuiltInCommands() {    // Register built-in commands that are always available
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
        startLogging(commandName); // Start logging for this command    
        commandParts.erase(commandParts.begin());   // Remove command name from arguments before passing to handler
        auto commandHandler = commandHandlerIterator->second;   //get pointer to the command function itself
        bool shouldContinueRunning = commandHandler(commandParts);  //execute command with arguments
        stopLogging(); // Stop logging after command completes
        
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

//
/*TeeStreambuf implementation*/ 
//

CommandDispatcher::TeeStreambuf::TeeStreambuf()
    : m_cout_buf(std::cout.rdbuf()), m_log_file(nullptr) {
}

CommandDispatcher::TeeStreambuf::~TeeStreambuf() {
    stopLogging();  // Ensure file is closed
    sync();    // Flush any remaining data just incase
}

void CommandDispatcher::TeeStreambuf::startLogging(const std::string& filepath) {
    stopLogging();  // Close any existing file
    m_log_file = std::make_unique<std::ofstream>(filepath, std::ios::app);
}

void CommandDispatcher::TeeStreambuf::stopLogging() {
    if (m_log_file && m_log_file->is_open()) {
        m_log_file->close();
    }
    m_log_file.reset();
}

int CommandDispatcher::TeeStreambuf::overflow(int c) {
    if (c == EOF) {
        return EOF;
    }
    if (m_cout_buf->sputc(c) == EOF) {    // Write to console
        return EOF;
    }
    if (m_log_file && m_log_file->is_open()) {    // Write to file
        m_log_file->put(c);
        m_log_file->flush();  // Immediate flush for real-time file output
    }
    return c;
}

int CommandDispatcher::TeeStreambuf::sync() {
    if (m_cout_buf->pubsync() == -1) {    // Sync console buffer
        return -1;
    }
    if (m_log_file && m_log_file->is_open()) {    // Sync file stream
        m_log_file->flush();
    }
    return 0;
}

void CommandDispatcher::startLogging(const std::string& commandName) {
    // timestamp and date of command
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto local_t = std::localtime(&time_t);
    
    // Create directories (does nothing if they already exist)
    std::stringstream dirpath;    
    dirpath << "logs/" << std::put_time(local_t, "%Y%m%d");
    std::filesystem::create_directories(dirpath.str());   

    std::stringstream filepath;    // Create full filepath
    filepath << dirpath.str() << "/" << commandName << "_" << std::put_time(local_t, "%H%M%S") << ".txt";
    
    // Tell TeeStreambuf to start logging to this file
    if (s_teeStreambuf) {
        s_teeStreambuf->startLogging(filepath.str());
    }
}

void CommandDispatcher::stopLogging() {
    if (s_teeStreambuf) {
        s_teeStreambuf->stopLogging();
    }
}

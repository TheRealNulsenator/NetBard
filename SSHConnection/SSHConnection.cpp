#include "SSHConnection.h"
#include <iostream>
#include <sstream>
#include <array>
#include <memory>
#include <cstdio>

SSHConnection::SSHConnection() {
}

SSHConnection::~SSHConnection() {
}

bool SSHConnection::testConnection(const std::string& hostname, const std::string& username, int port) {
    std::stringstream command;
    command << "ssh -o ConnectTimeout=5 -o BatchMode=yes -p " << port 
            << " " << username << "@" << hostname << " echo test 2>&1";
    
    std::string result = runSSHCommand(command.str());
    
    bool connectionSuccessful = (result.find("test") != std::string::npos);
    
    if (connectionSuccessful) {
        std::cout << "Successfully connected to " << username << "@" << hostname << std::endl;
    } else {
        std::cout << "Failed to connect to " << username << "@" << hostname << std::endl;
        if (!result.empty()) {
            std::cout << "Error: " << result << std::endl;
        }
    }
    
    return connectionSuccessful;
}

std::string SSHConnection::executeCommand(const std::string& hostname, const std::string& username, 
                                         const std::string& command, int port) {
    std::stringstream sshCommand;
    sshCommand << "ssh -o ConnectTimeout=5 -o BatchMode=yes -p " << port 
               << " " << username << "@" << hostname 
               << " \"" << command << "\" 2>&1";
    
    return runSSHCommand(sshCommand.str());
}

std::string SSHConnection::runSSHCommand(const std::string& sshCommand) {
    std::array<char, 128> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(sshCommand.c_str(), "r"), _pclose);
    
    if (!pipe) {
        return "Error: Failed to execute SSH command";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}

bool SSHConnection::handleCommand(const std::vector<std::string>& arguments) {
    if (arguments.size() < 3) {
        std::cout << "Usage: ssh <username> <hostname>" << std::endl;
        return true;
    }
    
    std::string username = arguments[1];
    std::string hostname = arguments[2];
    
    std::cout << "Testing SSH connection to " << username << "@" << hostname << "..." << std::endl;
    
    bool connectionSuccessful = testConnection(hostname, username);
    
    if (connectionSuccessful) {
        std::cout << "\nExecuting 'hostname' command..." << std::endl;
        std::string result = executeCommand(hostname, username, "hostname");
        std::cout << "Result: " << result << std::endl;
    }
    
    return true;  // Continue running
}
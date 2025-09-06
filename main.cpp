#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include "InputHandler.h"
#include "CommandDispatcher.h"
#include "SSHConnection.h"
#include "SubnetScanner.h"

const int MAIN_LOOP_DELAY_MS = 10;

int main() {
    std::cout << std::endl;
    std::cout << "Network Cartographer" << std::endl;
    std::cout << "Grantek - Brad Nulsen (2025)" << std::endl;
    std::cout << "\nType 'help' for commands, 'quit' to exit" << std::endl;
    std::cout << "> ";
    std::cout.flush();
    
    // Initialize major components
    InputHandler inputHandler;  // Starts collecting input immediately
    CommandDispatcher commandDispatcher; // Register callbacks to this
    SSHConnection sshConnection;
    SubnetScanner subnetScanner;
    
    // Register SSH command using std::bind
    commandDispatcher.registerCommand("ssh", 
        std::bind(&SSHConnection::handleCommand, &sshConnection, std::placeholders::_1),
        "Test SSH connection to a host");

    commandDispatcher.registerCommand("scan", 
        std::bind(&SubnetScanner::handleCommand, &subnetScanner, std::placeholders::_1),
        "search for alive hosts in a subnet");

    bool running = true;

    while (running) {    // Main loop

        if (inputHandler.hasCommand()) {
            std::string command = inputHandler.getCommand();
            running = commandDispatcher.processCommand(command);
            std::cout << "> ";
            std::cout.flush();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(MAIN_LOOP_DELAY_MS));
    }
    
    std::cout << "Exiting..." << std::endl;
    return 0;
}
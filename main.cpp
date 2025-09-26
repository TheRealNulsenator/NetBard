#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <winsock2.h>
#include "InputHandler.h"
#include "CommandDispatcher.h"
#include "SecureShell.h"
#include "PingScanner.h"
#include "TCPScanner.h"

const int MAIN_LOOP_DELAY_MS = 10;

int main() {
    // Initialize Winsock once for entire program
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "Network Cartographer" << std::endl;
    std::cout << "Brad Nulsen (2025)" << std::endl;
    std::cout << "\nType 'help' for commands, 'quit' to exit" << std::endl;
    std::cout << "> ";
    std::cout.flush();
    
    // Initialize major components
    InputHandler& inputHandler = InputHandler::getInstance();  // Starts collecting input immediately
    CommandDispatcher::initialize();  // Initialize built-in commands
    
    // Initialize tool commands (triggers auto-registration)
    SecureShell& secureShell = SecureShell::getInstance();
    PingScanner& pingScanner = PingScanner::getInstance();
    TCPScanner& tcpScanner = TCPScanner::getInstance();

    while (CommandDispatcher::s_running) {    // Main loop

        if (inputHandler.hasCommand()) {
            std::string command = inputHandler.getCommand();
            CommandDispatcher::processCommand(command);
            std::cout << "> ";
            std::cout.flush();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(MAIN_LOOP_DELAY_MS));
    }
    
    std::cout << "Exiting..." << std::endl;
    
    // Clean up Winsock
    WSACleanup();
    
    return 0;
}
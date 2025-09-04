#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "InputHandler.h"

bool processCommand(const std::string& command) {
    // Return false to quit, true to continue
    if (command == "quit" || command == "exit") {
        return false;
    }
    
    if (command == "help") {
        std::cout << "Available commands:" << std::endl;
        std::cout << "  help  - Show this message" << std::endl;
        std::cout << "  quit  - Exit the program" << std::endl;
        std::cout << "  exit  - Exit the program" << std::endl;
        return true;
    }
    
    // Add more commands here as needed
    std::cout << "Unknown command: " << command << std::endl;
    return true;
}

int main() {
    std::cout << "Network Cartographer" << std::endl;
    std::cout << "Grantek - Brad Nulsen (2025)" << std::endl;
    std::cout << "\nType 'help' for commands, 'quit' to exit" << std::endl;
    std::cout << "> ";
    std::cout.flush();
    
    InputHandler inputHandler;
    inputHandler.start();
    
    bool running = true;
    while (running) {
        if (inputHandler.hasCommand()) {
            std::string command = inputHandler.getCommand();
            running = processCommand(command);
            if (running) {
                std::cout << "> ";
                std::cout.flush();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    inputHandler.stop();
    std::cout << "Exiting..." << std::endl;
    return 0;
}
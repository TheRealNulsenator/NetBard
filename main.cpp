#include <iostream>
#include <thread>
#include <chrono>
#include "InputHandler.h"
#include "CommandDispatcher.h"

const int MAIN_LOOP_DELAY_MS = 10;

int main() {
    std::cout << "Network Cartographer" << std::endl;
    std::cout << "Grantek - Brad Nulsen (2025)" << std::endl;
    std::cout << "\nType 'help' for commands, 'quit' to exit" << std::endl;
    std::cout << "> ";
    std::cout.flush();
    
    // Initialize major components
    InputHandler inputHandler;
    CommandDispatcher commandDispatcher;
    
    inputHandler.start();
    

    bool running = true;

    while (running) {    // Main loop

        if (inputHandler.hasCommand()) {
            std::string command = inputHandler.getCommand();
            running = commandDispatcher.processCommand(command);

            if (running) {
                std::cout << "> ";
                std::cout.flush();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(MAIN_LOOP_DELAY_MS));
    }
    
    inputHandler.stop();
    std::cout << "Exiting..." << std::endl;
    return 0;
}
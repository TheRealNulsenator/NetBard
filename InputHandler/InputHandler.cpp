#include "InputHandler.h"
#include <iostream>
#include <chrono>

InputHandler::InputHandler() {
    m_inputThread = std::thread(&InputHandler::inputLoop, this);
    m_inputThread.detach();  // Fire and forget
}

InputHandler::~InputHandler() {
    // Don't even bother with stop() - just let process termination handle it
}

void InputHandler::inputLoop() {
    std::string input;
    while (true) {  //buffer inputs for lifetime of process
        std::getline(std::cin, input);
        if (!input.empty()) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_commands.push(input);
        }
    }
}

bool InputHandler::hasCommand() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_commands.empty();
}

std::string InputHandler::getCommand() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_commands.empty()) {
        return "";
    }
    std::string cmd = m_commands.front();
    m_commands.pop();
    return cmd;
}
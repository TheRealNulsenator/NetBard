#include "InputHandler.h"
#include <iostream>
#include <chrono>

InputHandler::InputHandler() : m_running(false) {
}

InputHandler::~InputHandler() {
    stop();
}

void InputHandler::start() {
    m_running = true;
    m_inputThread = std::thread(&InputHandler::inputLoop, this);
}

void InputHandler::stop() {
    m_running = false;

    if (m_inputThread.joinable()) {
        m_inputThread.join();
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

void InputHandler::inputLoop() {
    std::string input;

    while (m_running) {

        if (std::cin.peek() != EOF) {
            std::getline(std::cin, input);
            
            if (!input.empty()) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_commands.push(input);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
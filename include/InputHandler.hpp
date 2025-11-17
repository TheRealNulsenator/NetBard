#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>
#include <queue>
#include <thread>
#include <mutex>

class InputHandler { //SINGLETON
public:

    bool hasCommand();
    std::string getCommand();

    static InputHandler& getInstance() {    // Static method to get the single instance on the heap
        static InputHandler instance;       // Lazily initialized, thread-safe since C++11
        return instance;
    }

private:
    void inputLoop();

    std::queue<std::string> m_commands;
    std::mutex m_mutex;
    std::thread m_inputThread;

    // Private constructor to prevent direct instantiation
    InputHandler();
    ~InputHandler() {  }// just let process termination handle everything
    
    // Delete copy constructor and assignment operator to prevent copying
    InputHandler(const InputHandler&) = delete;
    InputHandler& operator=(const InputHandler&) = delete;
};

#endif // INPUT_HANDLER_H
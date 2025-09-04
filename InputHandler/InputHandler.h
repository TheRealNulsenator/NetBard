#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>

class InputHandler {
public:
    InputHandler();
    ~InputHandler();
    
    void start();
    void stop();
    bool hasCommand();
    std::string getCommand();
    
private:
    void inputLoop();
    
    std::queue<std::string> m_commands;
    std::mutex m_mutex;
    std::atomic<bool> m_running;
    std::thread m_inputThread;
};

#endif // INPUT_HANDLER_H
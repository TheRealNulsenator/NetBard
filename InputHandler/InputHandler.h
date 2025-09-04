#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>
#include <queue>
#include <thread>
#include <mutex>

class InputHandler {
public:
    InputHandler();
    ~InputHandler();

    bool hasCommand();
    std::string getCommand();
    
private:
    void inputLoop();
    
    std::queue<std::string> m_commands;
    std::mutex m_mutex;
    std::thread m_inputThread;
};

#endif // INPUT_HANDLER_H
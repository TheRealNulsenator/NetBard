#ifndef V_TOOL_COMMAND_H
#define V_TOOL_COMMAND_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "CommandDispatcher.h"
#include "LogStreambuf.h"

template<typename Derived>
class vToolCommand {
public:
    // derived classes must implement this
    virtual void handleCommand(const std::vector<std::string>& arguments) = 0;
    virtual void validateInput(const std::vector<std::string>& arguments) = 0;
    virtual ~vToolCommand() = default; //allow override but provide default
    
    // Delete copy constructor and assignment operator to enforce singleton pattern
    vToolCommand(const vToolCommand&) = delete;
    vToolCommand& operator=(const vToolCommand&) = delete;

    static Derived& getInstance() { // Singleton pattern with lazy registration
        static Derived instance;
        static bool registered = false;   

        if(!instance.m_log){ //create logger if we dont already have one 
            instance.m_log = std::make_unique<LogStreambuf>(Derived::COMMAND_PHRASE);
        }

        if (!registered) {  // Register on first use (two-phase initialization)
            CommandDispatcher::registerCommand( //safer to do this than risk it in the constructor 
                Derived::COMMAND_PHRASE,
                [](const std::vector<std::string>& args) { //opportunity to use decorator class pattern here
                    Derived &instance = Derived::getInstance();

                    instance.m_log->startLogging(args[0]);
                    instance.handleCommand(args);
                    instance.m_log->stopLogging();
                },
                Derived::COMMAND_TIP
            );
            registered = true;
        }      
        return instance;
    }
    
protected:

    std::unique_ptr<LogStreambuf> m_log; //instance of our logger

    vToolCommand() = default; 

    void save_results(const std::string& fileName, const std::string& data) {
        std::ofstream file(fileName);
        if (!file.is_open()) {
            std::cout << "failed to save output: " << fileName << std::endl;
            return;
        }
        else{
            file << data;
            file.close();
            std::cout << "saved output to: " << fileName << std::endl;
        } 
    }
    
private:

    friend Derived;    // Derived classes need access to protected constructor

};

#endif // V_TOOL_COMMAND_H
#ifndef V_TOOL_COMMAND_H
#define V_TOOL_COMMAND_H

#include <vector>
#include <string>
#include "CommandDispatcher.h"

template<typename Derived>
class vToolCommand {
public:
    // Virtual method that derived classes must implement
    virtual bool handleCommand(const std::vector<std::string>& arguments) = 0;

    virtual ~vToolCommand() = default;
    
    // Delete copy constructor and assignment operator for singleton
    vToolCommand(const vToolCommand&) = delete;
    vToolCommand& operator=(const vToolCommand&) = delete;

    // Singleton getInstance with lazy registration
    static Derived& getInstance() {
        static Derived instance;
        static bool registered = false;
        
        // Register on first use (two-phase initialization)
        if (!registered) { //I had to put this in because base constructors might run before vtable is made
            CommandDispatcher::registerCommand(
                Derived::COMMAND_PHRASE,
                [](const std::vector<std::string>& args) {
                    return Derived::getInstance().handleCommand(args);
                },
                Derived::COMMAND_TIP
            );
            registered = true;
        }
        
        return instance;
    }
    
protected:
    // Protected constructor for base class
    vToolCommand() = default;
    
private:
    // Derived classes need access to protected constructor
    friend Derived;
};

#endif // V_TOOL_COMMAND_H
#include <vector>
#include <string>
#include <CommandDispatcher.h>

class vToolCommand{ //intended as virtual function only

    public:

        virtual bool handleCommand(const std::vector<std::string>& arguments);
        virtual const std::string& getCommandPhrase() const = 0;
        virtual const std::string& getTipPhrase() const = 0;

        //this only works in modern C++, 
        vToolCommand(){ //vtable should be set up by the time base constructor is called
            CommandDispatcher::registerCommand(
                getCommandPhrase(), 
                std::bind(handleCommand, this, std::placeholders::_1),
                getTipPhrase());
        }

    protected:

    private:
        

};
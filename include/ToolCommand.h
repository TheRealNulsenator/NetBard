#include <vector>
#include <string>
#include <CommandDispatcher.h>

class ToolCommand{

    public:
        // Command handler for CommandDispatcher
        virtual bool handleCommand(const std::vector<std::string>& arguments);

    protected:

    private:
        

}
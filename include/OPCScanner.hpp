#ifndef OPC_EXPLORER_H
#define OPC_EXPLORER_H

#include "vToolCommand.hpp"

class OPCExplorer : public vToolCommand<OPCExplorer>{

    public:
        // Static command metadata for CRTP base class
        static constexpr const char* COMMAND_PHRASE = "opc";
        static constexpr const char* COMMAND_TIP = "Scan OPC node at designated path.\n\t\t\topc <address> <slot> <tagpath>";

        bool validateInput(const std::vector<std::string>& arguments) override;
        void handleCommand(const std::vector<std::string>& arguments) override;

    private:
        std::string _ip;
        int _slot;
        std::string _path;
        friend class vToolCommand<OPCExplorer>;
};


#endif
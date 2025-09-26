#ifndef TCP_SCANNER_H
#define TCP_SCANNER_H

#include "vToolCommand.h"
#include <string>
#include <vector>
#include <map>

class TCPScanner : public vToolCommand<TCPScanner>
{
    public:
        static constexpr const char* COMMAND_PHRASE = "tcp";
        static constexpr const char* COMMAND_TIP = "Scan TCP ports on target. Usage: tcp <ip>";

        static std::vector<int> Port_List;
        static std::map<int, std::string> Port_Descriptions;

        void handleCommand(const std::vector<std::string>& arguments) override;

    private:
        bool synHostPort(const std::string& host, const int port);
        TCPScanner();
        friend class vToolCommand<TCPScanner>;
};



#endif
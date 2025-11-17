#ifndef TCP_SCANNER_H
#define TCP_SCANNER_H

#include "vToolCommand.hpp"
#include <string>
#include <vector>
#include <map>

class TCPScanner : public vToolCommand<TCPScanner>
{
    public:
        static constexpr const char* COMMAND_PHRASE = "tcp";
        static constexpr const char* COMMAND_TIP = "Scan TCP ports on target.\n\t\ttcp <ip> \n\t\ttcp <cidr> <port>";

        static std::map<int, std::string> Ports;

        bool validateInput(const std::vector<std::string>& arguments) override;
        void handleCommand(const std::vector<std::string>& arguments) override;

    private:
        bool synHostPort(const std::string& host, const int port);
        TCPScanner();
        friend class vToolCommand<TCPScanner>;
};



#endif
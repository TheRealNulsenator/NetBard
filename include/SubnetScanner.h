#ifndef SUBNET_SCANNER_H
#define SUBNET_SCANNER_H

#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <winsock2.h>
#include <windows.h>
#include "vToolCommand.h"

class SubnetScanner : public vToolCommand<SubnetScanner>{

public:

    // Static command metadata for CRTP base class
    static constexpr const char* COMMAND_PHRASE = "scan";
    static constexpr const char* COMMAND_TIP = "search for active hosts in a subnet. Usage: scan <cidr>";

    bool pingHost(const std::string& address, HANDLE icmp_handle);
    bool handleCommand(const std::vector<std::string>& arguments) override;

    std::string Network_Address;
    std::string Broadcast_Address;
    std:: string Network_Mask;
    std::vector<std::string> Host_Addresses;
    std::map<std::string, bool> Host_Statuses;

private:

    std::vector<std::string> hosts;

    bool find_hosts();
    bool unwrap_cidr(const std::string& cidr, std::vector<std::string>& results);
    bool address_to_bits(const std::vector<std::string>& octets, uint32_t& ip);
    bool create_subnet_mask(const std::string& subnet_mask, uint32_t& results);
    std::string bits_to_address(const uint32_t ip);
};

#endif
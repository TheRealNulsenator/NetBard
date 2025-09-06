#ifndef SUBNET_SCANNER_H
#define SUBNET_SCANNER_H

#include <string>
#include <vector>
#include <cstdint>
#include <winsock2.h>
#include <windows.h>



class SubnetScanner {

public:
    bool pingHost(const std::string& address, HANDLE icmp_handle);
    bool handleCommand(const std::vector<std::string>& arguments);

private:

    std::string subnet_cidr;
    std::vector<std::string> subnet_hosts;

    const bool find_hosts(const std::string cidr);
    const bool unwrap_cidr(const std::string cidr, std::vector<std::string>& results);
    const bool address_to_bits(const std::vector<std::string> octets, uint32_t& ip);
    const bool create_subnet_mask(const std::string subnet_mask, uint32_t& results);
    const std::vector<uint32_t> create_address_range(const uint32_t ip, const uint32_t mask);
    const std::string bits_to_address(const uint32_t ip);
};

#endif
#ifndef SUBNET_PINGER_H
#define SUBNET_PINGER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>

class SubnetScanner {

public:
    //bool pingHost(const std::string address);
    bool handleCommand(const std::vector<std::string>& arguments);

private:

    std::string subnet_cider;
    std::vector<std::string> subnet_hosts;

    const bool create_host_list(const std::string cidr);
    const bool unwrap_cidr(const std::string cidr, std::vector<std::string>& results);
    const bool address_to_bits(const std::vector<std::string> octets, uint32_t& ip);
    const bool create_subnet_mask(const std::string subnet_mask, uint32_t& results);
    const std::vector<uint32_t> subnet_address_range(const uint32_t ip, const uint32_t mask);
};

#endif
#ifndef SUBNET_PINGER_H
#define SUBNET_PINGER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>

class SubnetScanner {

public:

    SubnetScanner();
    ~SubnetScanner();

    void scanSubnet(const std::string CIDR);
    bool pingHost(const std::string address);

private:

    std::string subnet_cider;
    std::vector<std::string> subnet_hosts;

    const bool create_host_list(const std::string cidr, std::vector<std::string>);
    const bool address_to_bits(const std::vector<std::string> octets, uint32_t& ip);
    const bool unwrap_cidr(const std::string cidr, std::vector<std::string>& results);

};

#endif
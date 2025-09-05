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

private:

    std::string subnet_cider;
    std::vector<std::string> subnet_hosts;

    bool pingHost(const std::string address);
    const std::vector<std::string> create_host_list(const std::string cidr);
    bool unwrap_cidr(const std::string cidr, std::vector<std::string>& results);
};

#endif
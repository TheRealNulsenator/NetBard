#ifndef SUBNET_PINGER_H
#define SUBNET_PINGER_H

#include <string>
#include <vector>

class SubnetScanner {
public:

void PingSubnet(const std::string CIDR);

private:

std::string subnet;
std::vector<std::string> ping_responders;

};

#endif
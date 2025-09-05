
#include "SubnetScanner.h"
#include <iostream>
#include <algorithm>
#include <bitset>
#include <cstdint>



bool SubnetScanner::handleCommand(const std::vector<std::string>& arguments) {

    if (arguments.size() == 1) {
        const std::string cidr = arguments[0];
        std::cout << "scanning subnet " << cidr << "..." << std::endl;
        find_hosts(cidr);
    }
    else{
        std::cout << "Usage: scan <cidr>" << std::endl;
        return true;
    }
    
    return true;
}

const bool SubnetScanner::find_hosts(const std::string cidr)
{

    std::vector<std::string> cidr_parts; //stores individual parts of subnet
    if (!unwrap_cidr(cidr, cidr_parts)) { std::cout << "Invalid CIDR (#.#.#.#/#)" << std::endl; return false;} 
   
    uint32_t ip; //binary address built of extracted octets
    if (!address_to_bits(cidr_parts, ip)) { std::cout << "Invalid Address" << std::endl; return false;}
        
    uint32_t mask; //extracts subnet mask shorthand into binary mask
    if (!create_subnet_mask(cidr_parts.back(), mask)) { std::cout << "Invalid Subnet" << std::endl; return false;}

    std::cout << std::bitset<32>(ip) << " " << std::bitset<32>(mask) << std::endl;
    std::vector<uint32_t> binary_addresses = create_address_range(ip, mask);
    
    return true;
}

const bool SubnetScanner::unwrap_cidr(const std::string cidr, std::vector<std::string>& results)
{
    results.clear();
    const std::string delimiters = "./\\";
    size_t start = 0;
    size_t end = cidr.find_first_of(delimiters);
    
    while (end != std::string::npos) {  //keep going until we have checked the whole string for delimiters
        
        if (end != start) {             //handles edge cases where first character is a delimiter, or consecutive delimiters
            results.push_back(cidr.substr(start, end - start));
        }
        start = end + 1;        //this blind pointer addition is how we introduce edge cases we need to check for above
        end = cidr.find_first_of(delimiters, start);
    }
    
    if (start < cidr.length()) { //shove the last segment onto the results vector
        results.push_back(cidr.substr(start));
    }

    const bool valid_octet_count = std::count(cidr.begin(), cidr.end(), '.') == 3;
    const bool valid_mask_count = std::count(cidr.begin(), cidr.end(), '/') == 1 || std::count(cidr.begin(), cidr.end(), '\\') == 1;
    const bool valid_token_count = results.size() == 5;
    //check for 4 total octets, and check for subnet with either forward or backslash
    return valid_octet_count && valid_mask_count && valid_token_count;
}

const bool SubnetScanner::address_to_bits(const std::vector<std::string> octets, uint32_t& ip)
{    
    ip = 0;  // Initialize the output parameter
    try {

        for (int i = 0; i < 4; i++) { //4 octets in a valid subnet. ignore anything else, like a dangling subnet mask
            int octet = std::stoi(octets[i]);

            if (octet < 0 || octet > 255) return false;  // Invalid octet range

            const uint8_t offset = (24 - (i * 8));
            ip |= (octet << offset);
        }
        return true;  // Success

    } catch (const std::exception& e) { // Conversion failed
        ip = 0; 
        return false;  
    }
}

const bool SubnetScanner::create_subnet_mask(const std::string subnet_mask, uint32_t& results) {
    int bits;
    try { bits = std::stoi(subnet_mask);} //perform string to int conversion
    catch(const std:: exception& e){ return false;}

    if (bits < 0 || bits > 32) return false;    // Invalid input
    else if (bits == 0) results = 0;              // All zeros
    else if (bits == 32) results = 0xFFFFFFFF;    // All ones
    else results = 0xFFFFFFFF << (32 - bits);   // no edge case, perform normal conversion

    return true;
}


const std::vector<uint32_t> SubnetScanner::create_address_range(const uint32_t ip, const uint32_t mask)
{

    const uint32_t network_address = ip & mask;
    const uint32_t broadcast_address = ip | ~mask;

    std::cout << "Network: " << std::bitset<32>(network_address) << "    " << "Broadcast: " << std::bitset<32>(broadcast_address)  << std::endl;

    std::vector<uint32_t> addresses;
    for (uint32_t address = network_address + 1; address < broadcast_address; address++){
        addresses.push_back(address);
    }

    std::cout << "Unique Addresses: " << addresses.size() << std::endl;

    return addresses;
}
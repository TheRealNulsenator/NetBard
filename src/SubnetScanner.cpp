
#include "SubnetScanner.h"
#include <string>
#include <iostream>


const bool SubnetScanner::create_host_list(const std::string cidr, std::vector<std::string>)
{
    const std::vector<std::string> hosts;

    std::vector<std::string> cidr_parts; //stores individual parts of subnet
    if (!unwrap_cidr(cidr, cidr_parts))  std::cout << "Invalid CIDR (###.###.###.###/#)"; return false; 
   
    uint32_t ip;
    if (!address_to_bits(cidr_parts, ip)) std::cout << "Invalid Address" << std::endl; return false;
        
    
    return true;
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
    return  valid_octet_count && valid_mask_count && valid_token_count;
}


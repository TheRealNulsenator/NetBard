
#include "SubnetScanner.h"
#include <string>
#include <iostream>


const std::vector<std::string> SubnetScanner::create_host_list(const std::string cidr)
{
    const std::vector<std::string> hosts;
    std::vector<std::string> cidr_parts; //stores individual parts of subnet
    
    if (unwrap_cidr(cidr, cidr_parts)){ //if we have a valid subnet, begin processing it into hosts
        
    }
    else{
        std::cout << "Invalid CIDR (###.###.###.###/#)" << std::endl;
    }


}


bool SubnetScanner::unwrap_cidr(const std::string cidr, std::vector<std::string>& results)
{
    const std::string delimiters = "./\\";
    std::vector<std::string> tokens; //stores divided subcomponents of CIDR
    size_t start = 0;
    size_t end = cidr.find_first_of(delimiters);
    
    while (end != std::string::npos) {  //keep going until we have checked the whole string for delimiters
        
        if (end != start) {             //handles edge cases where first character is a delimiter, or consecutive delimiters
            tokens.push_back(cidr.substr(start, end - start));
        }
        start = end + 1;        //this blind pointer addition is how we introduce edge cases we need to check for above
        end = cidr.find_first_of(delimiters, start);
    }
    
    if (start < cidr.length()) { //shove the last segment on
        tokens.push_back(cidr.substr(start));
    }

    results.clear();
    results.resize(tokens.size());
    if (!tokens.empty()) {
        memcpy(results.data(), tokens.data(), tokens.size() * sizeof(std::string));
    }

    const bool valid_octet_count = std::count(cidr.begin(), cidr.end(), '.') == 3;
    const bool valid_mask_count = std::count(cidr.begin(), cidr.end(), '/') == 1 || std::count(cidr.begin(), cidr.end(), '\\') == 1;
    const bool valid_token_count = tokens.size() == 5;
    //check for 4 total octets, and check for subnet with either forward or backslash
    return  valid_octet_count && valid_mask_count && valid_token_count;
}


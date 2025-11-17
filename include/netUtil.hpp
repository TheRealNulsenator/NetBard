#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#include <vector>
#include <sstream>

namespace netUtil {

    // Validate IPv4 address format using inet_pton
    inline bool isValidIPv4(const std::string& ip) {
        struct sockaddr_in sa;
        return inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) == 1;
    }

    // Validate port number is in valid range
    inline bool isValidPort(int port) {
        return port >= 1 && port <= 65535;
    }
    inline bool isValidPort(std::string port){
        int port_num = stoi(port);
        return port_num >= 1 && port_num <= 65535;
    }

    // Validate CIDR notation (e.g., 192.168.1.0/24)
    inline bool isValidCIDR(const std::string& cidr) {
        // Check for exactly one slash
        size_t slash_count = std::count(cidr.begin(), cidr.end(), '/');
        size_t backslash_count = std::count(cidr.begin(), cidr.end(), '\\');

        if (slash_count + backslash_count != 1) {
            return false;
        }

        // Find delimiter position
        size_t delimiter_pos = cidr.find('/');
        if (delimiter_pos == std::string::npos) {
            delimiter_pos = cidr.find('\\');
        }

        // Extract IP and mask parts
        std::string ip_part = cidr.substr(0, delimiter_pos);
        std::string mask_part = cidr.substr(delimiter_pos + 1);

        // Validate IP portion
        if (!isValidIPv4(ip_part)) {
            return false;
        }

        // Validate mask is a number between 0 and 32
        try {
            int mask = std::stoi(mask_part);
            return mask >= 0 && mask <= 32;
        } catch (...) {
            return false;
        }
    }

    // Parse CIDR into components (returns empty vector on failure)
    inline std::vector<std::string> parseCIDR(const std::string& cidr) {
        std::vector<std::string> parts;
        const std::string delimiters = "./\\";
        size_t start = 0;
        size_t end = cidr.find_first_of(delimiters);
        
        while (end != std::string::npos) {  //keep going until we have checked the whole string for delimiters
            
            if (end != start) {             //handles edge cases where first character is a delimiter, or consecutive delimiters
                parts.push_back(cidr.substr(start, end - start));
            }
            start = end + 1;        //this blind pointer addition is how we introduce edge cases we need to check for above
            end = cidr.find_first_of(delimiters, start);
        }
        
        if (start < cidr.length()) { //shove the last segment onto the results vector
            parts.push_back(cidr.substr(start));
        }

        constexpr int EXPECTED_DOTS = 3;
        constexpr size_t EXPECTED_TOKENS = 5;  // 4 octets + 1 mask
        const bool valid_octet_count = std::count(cidr.begin(), cidr.end(), '.') == EXPECTED_DOTS;
        const bool valid_mask_count = std::count(cidr.begin(), cidr.end(), '/') == 1 || std::count(cidr.begin(), cidr.end(), '\\') == 1;
        const bool valid_token_count = parts.size() == EXPECTED_TOKENS;
        if(!(valid_octet_count)){
            parts.clear();
        }
        return parts;
    }

    // Convert IP string to binary representation
    inline bool ipToBinary(const std::string& ip, uint32_t& binary) {
        struct sockaddr_in sa;
        if (inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) == 1) {
            binary = ntohl(sa.sin_addr.s_addr);
            return true;
        }
        return false;
    }

    // Convert binary IP to string representation
    inline std::string binaryToIP(uint32_t ip) {
        struct in_addr addr;
        addr.s_addr = htonl(ip);
        char buffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr, buffer, INET_ADDRSTRLEN);
        return std::string(buffer);
    }

    // Check if string contains only digits
    inline bool isNumeric(const std::string& str) {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    }

    // Validate hostname (basic check - alphanumeric, dots, hyphens)
    inline bool isValidHostname(const std::string& hostname) {
        if (hostname.empty() || hostname.length() > 255) {
            return false;
        }

        return std::all_of(hostname.begin(), hostname.end(), [](char c) {
            return std::isalnum(c) || c == '.' || c == '-';
        });
    }


    inline bool octets_to_bits(const std::vector<std::string>& octets, uint32_t& ip)
    {    
        ip = 0;  // Initialize the output parameter
        try {

            const int IPV4_OCTETS = 4;
            for (int i = 0; i < IPV4_OCTETS; i++) { //octets in a valid subnet. ignore anything else, like a dangling subnet mask
                int octet = std::stoi(octets[i]);

                const int MIN_OCTET = 0;
                const int MAX_OCTET = 255;
                if (octet < MIN_OCTET || octet > MAX_OCTET) return false;  // Invalid octet range

                const uint8_t BITS_PER_OCTET = 8;
                const uint8_t offset = (24 - (i * BITS_PER_OCTET));
                ip |= (octet << offset);
            }
            return true;  // Success

        } catch (const std::exception& e) { // Conversion failed
            ip = 0; 
            return false;  
        }
    }


    inline std::string bits_to_address(const uint32_t ip)
    {
        std::stringstream ss; //shift for each octet!
        ss << ((ip >> 24) & 0xFF) << "."
        << ((ip >> 16) & 0xFF) << "."
        << ((ip >> 8) & 0xFF) << "."
        << (ip & 0xFF);
        return ss.str();
    }


    inline bool mask_to_bits(const std::string& subnet_mask, uint32_t& results) {
        int bits;
        try { bits = std::stoi(subnet_mask);} //perform string to int conversion
        catch(const  std::exception& e){ return false;}

        const int MAX_SUBNET_BITS = 32;
        if (bits < 0 || bits > MAX_SUBNET_BITS) return false;    // Invalid input
        else results = 0xFFFFFFFF << (MAX_SUBNET_BITS - bits);   // no edge case, perform normal conversion

        return true;
    }


}

#endif
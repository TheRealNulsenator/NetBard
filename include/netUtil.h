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
}

#endif // NETWORK_VALIDATION_H
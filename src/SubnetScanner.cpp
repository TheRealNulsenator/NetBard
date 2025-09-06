
#include "SubnetScanner.h"
#include <iostream>
#include <algorithm>
#include <bitset>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <semaphore>
#include <iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

bool SubnetScanner::handleCommand(const std::vector<std::string>& arguments) {

    if (arguments.size() == 0){
        std::cout << "Usage: scan <cidr>" << std::endl;
        return true;
    }

    if (arguments.size() >= 1) {
        const std::string subnet_cidr = arguments[0];
        find_hosts(subnet_cidr);
    }

    
    return true;
}

const bool SubnetScanner::find_hosts(const std::string cidr)
{
    subnet_cidr = cidr;  // Store the CIDR for later use
    subnet_hosts.clear();  // Clear previous scan results

    std::cout << "scanning subnet " << subnet_cidr << " for hosts..." << std::endl;
    std::vector<std::string> cidr_parts; //stores individual parts of subnet
    if (!unwrap_cidr(cidr, cidr_parts)) { std::cout << "Invalid CIDR (#.#.#.#/#)" << std::endl; return false;} 
   
    uint32_t ip; //binary address built of extracted octets
    if (!address_to_bits(cidr_parts, ip)) { std::cout << "Invalid Address" << std::endl; return false;}
        
    uint32_t mask; //extracts subnet mask shorthand into binary mask
    if (!create_subnet_mask(cidr_parts.back(), mask)) { std::cout << "Invalid Subnet" << std::endl; return false;}

    std::vector<uint32_t> binary_addresses = create_address_range(ip, mask);
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {    // initialize Winsock once for all pings
        std::cout << "Failed to initialize Winsock" << std::endl;
        return false;
    }


    const auto MAX_THREADS = 100; //max concurrent pings
    const auto MS_BETWEEN_PINGS = 10; //delay between pings

    std::cout << "\nScanning hosts..." << std::endl;
    std::vector<std::thread> threads;
    std::mutex output_mutex; //dont try to all talk at once

    for (const uint32_t& binary_ip : binary_addresses) {

        HANDLE icmp_handle = IcmpCreateFile();    // create ICMP handle once for each thread
        if (icmp_handle == INVALID_HANDLE_VALUE) {
            std::cout << "Failed to create ICMP handle" << std::endl;
            return false;
        }

        //uses emplace back to avoid create then copy, which would spin up 2 threads
        threads.emplace_back([&, binary_ip](HANDLE handle) { 
            std::string address = bits_to_address(binary_ip);
            
            if (pingHost(address, handle)) { //critical code section that can block other threads
                std::lock_guard<std::mutex> lock(output_mutex);
                std::cout << address << " is alive" << std::endl;
                subnet_hosts.push_back(address);          
            }
                
        }, icmp_handle);
        // Delay how quickly we spin up threads to avoid faulting old PLC processors win ping storm
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
        
    // Wait for remaining threads
    for (auto& thread : threads) {
        thread.join();
    }

    // clean up ICMP handle and Winsock

    WSACleanup();
    
    std::cout << "\nScan complete. Found " << subnet_hosts.size() << " alive hosts." << std::endl;
    
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


const std::string SubnetScanner::bits_to_address(const uint32_t ip)
{
    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << "."
       << ((ip >> 16) & 0xFF) << "."
       << ((ip >> 8) & 0xFF) << "."
       << (ip & 0xFF);
    return ss.str();
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

    std::cout << "Network:      " << std::bitset<32>(network_address) << std::endl;
    std::cout << "Mask:         " << std::bitset<32>(mask) << std::endl;
    std::cout << "Broadcast:    " << std::bitset<32>(broadcast_address)  << std::endl;

    std::vector<uint32_t> addresses;
    for (uint32_t address = network_address + 1; address < broadcast_address; address++){
        addresses.push_back(address);
    }

    std::cout << "Unique Addresses: " << addresses.size() << std::endl;

    return addresses;
}


bool SubnetScanner::pingHost(const std::string& address, HANDLE icmp_handle)
{
    const int PING_TIMEOUT_MS = 500; 
    const int MAX_TRIES = 4;
    auto ping_attempts = 0;

    while (ping_attempts < MAX_TRIES){
        ping_attempts++;

        // Convert string IP to network address
        unsigned long dest_addr = inet_addr(address.c_str());
        if (dest_addr == INADDR_NONE) {
            return false;
        }

        // prepare memory for windows to send ping and receive response
        const char send_data[] = "ping";
        const DWORD reply_size = sizeof(ICMP_ECHO_REPLY) + sizeof(send_data);
        LPVOID reply_buffer = (VOID*) malloc(reply_size);
        if (reply_buffer == nullptr) {
            return false;
        }
        
        // std::cout << "pinging " << address << ", attempt " << ping_attempts << "\r" << std::flush;

        // send ICMP echo request
        DWORD reply_count = IcmpSendEcho( //THIS IS A BLOCKING FUNCTION!
            icmp_handle,
            dest_addr,
            (LPVOID)send_data,
            sizeof(send_data),
            nullptr,
            reply_buffer,
            reply_size,
            PING_TIMEOUT_MS //blocks for at most this long
        );

        bool host_alive = false;
        if (reply_count > 0) { //check struct returned for success enum
            PICMP_ECHO_REPLY echo_reply = (PICMP_ECHO_REPLY)reply_buffer;
            if (echo_reply->Status == IP_SUCCESS) {

                host_alive = true;
            }
        }
        free(reply_buffer); //we manually allocated, clean it up!
        
        if (host_alive) {
            IcmpCloseHandle(icmp_handle);
            return true; //we got a ping, break out of loop
        }
    }
    IcmpCloseHandle(icmp_handle);
    return false;
    
}
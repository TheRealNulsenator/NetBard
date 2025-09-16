
#include "SubnetScanner.h"
#include <iostream>
#include <algorithm>
#include <bitset>
#include <sstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

void SubnetScanner::handleCommand(const std::vector<std::string>& arguments) {

    if (arguments.size() == 0){
        std::cout << "Usage: scan <cidr>" << std::endl;
        return;
    }

    if (arguments.size() >= 1) {
        const std::string subnet_cidr = arguments[0];

        std::vector<std::string> cidr_parts; //stores individual parts of subnet
        if (!unwrap_cidr(subnet_cidr, cidr_parts)) { std::cout << "Invalid CIDR (#.#.#.#/#)" << std::endl; return;} 
    
        uint32_t ip; //binary address built of extracted octets
        if (!address_to_bits(cidr_parts, ip)) { std::cout << "Invalid Address" << std::endl; return;}
            
        uint32_t mask; //extracts subnet mask shorthand into binary mask
        if (!create_subnet_mask(cidr_parts.back(), mask)) { std::cout << "Invalid Subnet" << std::endl; return;}

        const uint32_t network_address = ip & mask;
        const uint32_t broadcast_address = ip | ~mask;

        std::cout << "Network:      " << std::bitset<32>(network_address) << std::endl;
        std::cout << "Mask:         " << std::bitset<32>(mask) << std::endl;
        std::cout << "Broadcast:    " << std::bitset<32>(broadcast_address)  << std::endl;

        Network_Address = bits_to_address(network_address);
        Broadcast_Address = bits_to_address(broadcast_address);
        Host_Addresses.clear();  // Clear previous scan results
        for (uint32_t bit_address = network_address + 1; bit_address < broadcast_address; bit_address++){
            std::string string_address = bits_to_address(bit_address);
            Host_Addresses.push_back(string_address);
        }

        std::cout << "Unique addresses: " << Host_Addresses.size() << std::endl;
        find_hosts();
    }
}

bool SubnetScanner::find_hosts()
{
    std::cout << "Scanning subnet " << Network_Address << " for hosts..." << std::endl;

    Host_Statuses.clear(); //reset status keys


    std::atomic<int> index_of_next_address_to_ping = 0; //atomic, so that threads dont try to access same index
    std::vector<std::thread> threads;
    std::mutex output_mutex; //dont try to all talk at once

    const auto MAX_THREADS = 100;               //max concurrent pinging threads
    const auto MS_BETWEEN_THREAD_SPAWNS = 10;   //small delay between thread spawns to protect old PLCs
    for (int i = 0; i < MAX_THREADS; i++) {     //spawn all of our threads!

        //uses emplace_back to avoid attempting thread copy operation
        threads.emplace_back([&]() -> bool {        //used lambda because this is an over engineered solution
            HANDLE icmp_handle = IcmpCreateFile();  // create ICMP handle once for each thread
            if (icmp_handle == INVALID_HANDLE_VALUE) {
                std::cout << "Failed to create ICMP handle" << std::endl;
                return true;
            } 

            while (true) {      //keep pinging addresses until we have gotten them all
                int my_index = index_of_next_address_to_ping.fetch_add(1); //ATOMIC fetch and increment
                if (my_index >= Host_Addresses.size()) break;   //no more work
                
                std::string address = Host_Addresses[my_index];
                bool pingable = pingHost(address, icmp_handle);
                {   //CRITICAL SECTION that can block other threads from accessing this while we write status
                    std::lock_guard<std::mutex> lock(output_mutex);
                    if (pingable) {
                        std::cout << address << " is alive" << std::endl;
                    }
                    Host_Statuses[address] = pingable;   
                }  
            }
            IcmpCloseHandle(icmp_handle);  
            return true;      
        });
        // Delay how quickly we spin up threads to avoid faulting old PLC processors with ping storm
        std::this_thread::sleep_for(std::chrono::milliseconds(MS_BETWEEN_THREAD_SPAWNS));
    }
        
    for (auto& thread : threads) {    // Wait for all threads to terminate
        thread.join();
    }

    int alive_count = 0;
    for (const auto& [address, is_alive] : Host_Statuses) {
        if (is_alive) alive_count++;
    }
    
    std::cout << "Scan complete. Found " << alive_count << " alive hosts out of " << Host_Addresses.size() << " scanned." << std::endl;
    
    return true;
}


bool SubnetScanner::unwrap_cidr(const std::string& cidr, std::vector<std::string>& results)
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

    const int EXPECTED_DOTS = 3;
    const bool valid_octet_count = std::count(cidr.begin(), cidr.end(), '.') == EXPECTED_DOTS;
    const bool valid_mask_count = std::count(cidr.begin(), cidr.end(), '/') == 1 || std::count(cidr.begin(), cidr.end(), '\\') == 1;
    const size_t EXPECTED_TOKENS = 5;  // 4 octets + 1 mask
    const bool valid_token_count = results.size() == EXPECTED_TOKENS;
    //check for 4 total octets, and check for subnet with either forward or backslash
    return valid_octet_count && valid_mask_count && valid_token_count;
}


bool SubnetScanner::address_to_bits(const std::vector<std::string>& octets, uint32_t& ip)
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


std::string SubnetScanner::bits_to_address(const uint32_t ip)
{
    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << "."
       << ((ip >> 16) & 0xFF) << "."
       << ((ip >> 8) & 0xFF) << "."
       << (ip & 0xFF);
    return ss.str();
}


bool SubnetScanner::create_subnet_mask(const std::string& subnet_mask, uint32_t& results) {
    int bits;
    try { bits = std::stoi(subnet_mask);} //perform string to int conversion
    catch(const std:: exception& e){ return false;}

    const int MAX_SUBNET_BITS = 32;
    if (bits < 0 || bits > MAX_SUBNET_BITS) return false;    // Invalid input
    else results = 0xFFFFFFFF << (MAX_SUBNET_BITS - bits);   // no edge case, perform normal conversion

    return true;
}


bool SubnetScanner::pingHost(const std::string& address, HANDLE icmp_handle)
{
    const int PING_TIMEOUT_MS = 2000;
    const int MAX_PING_ATTEMPTS = 2;
    auto ping_attempts = 0;

    while (ping_attempts < MAX_PING_ATTEMPTS){
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
            return true; //we got a ping, break out of loop
        }
    }
    return false;
}
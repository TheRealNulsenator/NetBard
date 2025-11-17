#include "PingScanner.hpp"
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
#include <netUtil.hpp>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

PingScanner::PingScanner(){}


bool PingScanner::validateInput(const std::vector<std::string>& arguments){

    m_cidr_parts.clear();
    switch(arguments.size()){
        case 0:
            return false;
            break;
        case 1:
            if(netUtil::isValidCIDR(arguments[0])){
                m_cidr_parts = netUtil::parseCIDR(arguments[0]);
                return true;
            }
            else if (netUtil::isValidIPv4(arguments[0])){
                m_cidr_parts = netUtil::parseCIDR(arguments[0]);
                m_cidr_parts.push_back("32");
                return true;
            }
            else{
                return false;
            }
            break;
        default:
            return false;
    }

}

void PingScanner::handleCommand(const std::vector<std::string>& arguments) {

    uint32_t ip; //binary address built of extracted octets
    if (!netUtil::octets_to_bits(m_cidr_parts, ip)) { std::cout << "Invalid Address" << std::endl; return;}

    uint32_t mask; //extracts subnet mask shorthand into binary mask
    if (!netUtil::mask_to_bits(m_cidr_parts.back(), mask)) { std::cout << "Invalid Subnet" << std::endl; return;}

    if(mask == UINT32_MAX){
        std::string host_address = netUtil::bits_to_address(ip);
        std::cout << "Pinging host: " << host_address << std::endl;
        HANDLE icmp_handle = IcmpCreateFile();
        if (icmp_handle == INVALID_HANDLE_VALUE) { std::cout << "Failed to create ICMP handle" << std::endl; return;}
        if(pingHost(host_address, icmp_handle)){
            std::cout << "Responded!" << std::endl;
        }
        else{
            std::cout << "No response." << std::endl;
        }
    }
    else{
        scan(ip, mask);
    }
}


void PingScanner::scan(uint32_t ip, uint32_t mask){

    const uint32_t network_address = ip & mask;
    const uint32_t broadcast_address = ip | ~mask;

    std::cout << "Network:      " << std::bitset<32>(network_address) << std::endl;
    std::cout << "Mask:         " << std::bitset<32>(mask) << std::endl;
    std::cout << "Broadcast:    " << std::bitset<32>(broadcast_address)  << std::endl;
    Network_Address = netUtil::bits_to_address(network_address);
    Broadcast_Address = netUtil::bits_to_address(broadcast_address);
    Host_Addresses.clear();  // Clear previous scan results
    for (uint32_t bit_address = network_address + 1; bit_address < broadcast_address; bit_address++){
        std::string string_address = netUtil::bits_to_address(bit_address);
        Host_Addresses.push_back(string_address);
    }

    std::cout << "Unique addresses: " << Host_Addresses.size() << std::endl;
    std::cout << "Scanning " << Network_Address << " via Ping..." << std::endl;

    Host_Statuses.clear(); //reset status keys

    std::atomic<int> index_of_next_address_to_ping = 0; //atomic, so that threads dont try to access same index
    std::vector<std::thread> threads;
    std::mutex output_mutex; //dont try to all talk at once

    //workstealing thread pool to ping all the IPs
    const auto MAX_THREADS = 100;               //max concurrent pinging threads
    const auto MS_BETWEEN_THREAD_SPAWNS = 10;   //small delay between thread spawns to protect old PLCs
    for (int i = 0; i < MAX_THREADS; i++) {     //spawn all of our threads!
        //uses emplace_back to avoid attempting thread copy operation
        threads.emplace_back([&]() -> void {  //used lambda because this is an over-engineered solution
            HANDLE icmp_handle = IcmpCreateFile();  // create ICMP handle once for each thread
            //ICMP is stateful connection, handle=special file that stores connection state
            if (icmp_handle == INVALID_HANDLE_VALUE) {std::cout << "Failed to create ICMP handle" << std::endl;return;}
            while (true) {  //keep scanning addresses until we have gotten them all and main program closes them all.
                int my_index = index_of_next_address_to_ping.fetch_add(1); //ATOMIC fetch and increment (implicit mutex usage with minimized critical code section)
                if (my_index >= Host_Addresses.size()) break;   //no more work
                std::string address = Host_Addresses[my_index];
                bool pingable = pingHost(address, icmp_handle);
                {   //CRITICAL SECTION that can block other threads from accessing this while we write status
                    std::lock_guard<std::mutex> lock(output_mutex);
                    if (pingable) {
                        std::cout << address << std::endl;
                    }
                    Host_Statuses[address] = pingable;
                }
            }
            IcmpCloseHandle(icmp_handle);
            //END OF LAMBDA
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


}

bool PingScanner::pingHost(const std::string& address, HANDLE icmp_handle)
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
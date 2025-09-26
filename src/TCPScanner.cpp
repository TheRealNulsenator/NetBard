
#include "TCPScanner.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <algorithm>
#include <map>
#include <thread>
#include <chrono>

std::vector<int> TCPScanner::Port_List = {
    // Web services
    80,     // HTTP
    443,    // HTTPS
    8080,   // HTTP Alternate
    8443,   // HTTPS Alternate

    // Industrial protocols
    502,    // Modbus TCP
    102,    // Siemens S7 / IEC 61850
    44818,  // EtherNet/IP
    2222,   // EtherNet/IP I/O
    4840,   // OPC UA
    4843,   // OPC UA with TLS
    47808,  // BACnet
    20000,  // DNP3
    1883,   // MQTT
    8883,   // MQTT with TLS

    // Network management
    161,    // SNMP
    162,    // SNMP Trap
    22,     // SSH
    23,     // Telnet
    21,     // FTP Control
    20,     // FTP Data
    69,     // TFTP

    // Database & HMI
    1433,   // MS SQL Server
    3306,   // MySQL
    5432,   // PostgreSQL
    5900,   // VNC
    3389,   // RDP

    // Other industrial
    9600,   // OMRON FINS
    5000,   // Siemens S7 (alternate)
    5001,   // Siemens S7 (alternate)
    1911,   // Niagara Fox
    1962,   // PCWorx
    789,    // Crimson v3
    10001,  // Ubiquiti Discovery
    2455,   // WAGO CoDeSys
    34962,  // Profinet
    34963,  // Profinet
    34964,  // Profinet
    2404,   // IEC 60870-5-104

    // Email (often on industrial servers)
    25,     // SMTP
    110,    // POP3
    143,    // IMAP
    587,    // SMTP (submission)
    993,    // IMAPS
    995     // POP3S
};

// Port descriptions map
std::map<int, std::string> TCPScanner::Port_Descriptions = {
    // Web services
    {80, "HTTP Web Server"},
    {443, "HTTPS Secure Web Server"},
    {8080, "HTTP Alternate"},
    {8443, "HTTPS Alternate"},

    // Industrial protocols
    {502, "Modbus TCP"},
    {102, "Siemens S7 / IEC 61850"},
    {44818, "EtherNet/IP Explicit Messaging"},
    {2222, "EtherNet/IP I/O Data"},
    {4840, "OPC UA"},
    {4843, "OPC UA with TLS"},
    {47808, "BACnet/IP"},
    {20000, "DNP3"},
    {1883, "MQTT"},
    {8883, "MQTT with TLS"},

    // Network management
    {161, "SNMP"},
    {162, "SNMP Trap"},
    {22, "SSH Secure Shell"},
    {23, "Telnet"},
    {21, "FTP Control"},
    {20, "FTP Data"},
    {69, "TFTP"},

    // Database & HMI
    {1433, "Microsoft SQL Server"},
    {3306, "MySQL Database"},
    {5432, "PostgreSQL Database"},
    {5900, "VNC Remote Desktop"},
    {3389, "Windows RDP"},

    // Other industrial
    {9600, "OMRON FINS"},
    {5000, "Siemens S7 (alternate)"},
    {5001, "Siemens S7 (alternate)"},
    {1911, "Niagara Fox Protocol"},
    {1962, "Phoenix Contact PCWorx"},
    {789, "Red Lion Crimson v3"},
    {10001, "Ubiquiti Discovery"},
    {2455, "WAGO CoDeSys"},
    {34962, "Profinet RT"},
    {34963, "Profinet RT"},
    {34964, "Profinet RT"},
    {2404, "IEC 60870-5-104"},

    // Email
    {25, "SMTP Mail"},
    {110, "POP3 Mail"},
    {143, "IMAP Mail"},
    {587, "SMTP Submission"},
    {993, "IMAP Secure"},
    {995, "POP3 Secure"}
};


TCPScanner::TCPScanner() {}

// Handle command implementation
void TCPScanner::handleCommand(const std::vector<std::string>& arguments) {
    if (arguments.size() < 1) {
        std::cout << "Usage: tcp <ip_address>" << std::endl;
        return;
    }

    std::string target_ip = arguments[0];
    std::cout << "Scanning ports on " << target_ip << std::endl;

    std::for_each(Port_List.begin(), Port_List.end(), [this, &target_ip](int port){
        synHostPort(target_ip, port);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    });
}


// TCP Connect Scan - attempts full TCP connection to test if port is open
bool TCPScanner::synHostPort(const std::string& host, const int port) {
    // Create a TCP socket (like opening a phone line)
    SOCKET tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_socket == INVALID_SOCKET) {
        return false;
    }

    // Make socket non-blocking so connect() won't freeze our program
    u_long non_blocking_mode = 1;
    ioctlsocket(tcp_socket, FIONBIO, &non_blocking_mode);

    // Build the target address structure (like writing an address on an envelope)
    sockaddr_in target_address;
    target_address.sin_family = AF_INET;                              // IPv4 address type
    target_address.sin_port = htons(port);                            // Convert port to network byte order
    inet_pton(AF_INET, host.c_str(), &target_address.sin_addr);      // Convert IP string to binary

    // Start connection attempt (like dialing a phone number)
    // Returns immediately due to non-blocking mode
    int connect_result = connect(tcp_socket, (sockaddr*)&target_address, sizeof(target_address));

    // Prepare sets of sockets to monitor
    fd_set sockets_that_connected;     // Will contain sockets that successfully connected
    fd_set sockets_with_errors;        // Will contain sockets that had errors
    FD_ZERO(&sockets_that_connected);  // Clear the set
    FD_ZERO(&sockets_with_errors);     // Clear the set
    FD_SET(tcp_socket, &sockets_that_connected);  // Add our socket to monitor
    FD_SET(tcp_socket, &sockets_with_errors);     // Add our socket to monitor

    // Set how long we'll wait for connection
    timeval max_wait_time;
    max_wait_time.tv_sec = 0;
    max_wait_time.tv_usec = 500000;  // 500 milliseconds

    // Wait for connection to succeed or fail (like waiting for phone to ring or get busy signal)
    int select_result = select(0, nullptr, &sockets_that_connected, &sockets_with_errors, &max_wait_time);

    bool port_is_open = false;
    if (select_result > 0) {
        // Check if our socket successfully connected
        if (FD_ISSET(tcp_socket, &sockets_that_connected)) {
            port_is_open = true;

            // Look up what service typically runs on this port
            auto service_lookup = Port_Descriptions.find(port);
            std::string service_name = (service_lookup != Port_Descriptions.end())
                                      ? service_lookup->second
                                      : "Unknown Service";
            std::cout << "  Port " << port << " OPEN - " << service_name << std::endl;
        }
    }

    closesocket(tcp_socket);
    return port_is_open;
}



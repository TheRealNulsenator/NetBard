#include "SSHConnection.h"
#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <libssh2.h>
#include <thread>
#include <chrono>

const std::vector<std::string> SSHConnection::DISCOVERY_COMMANDS = {
    "terminal length 0",                        // Return full output of all subsequent commands without dialogue
    "show version",                            // Device model, IOS version, uptime
    "show ip interface brief",                 // All interfaces with IP addresses
    "show interfaces description",             // Interface descriptions
    "show interface status",                   // Basic interface configurations
    "show mac address-table",                  // All layer 2 traffic
    "show ip route",                           // Routing table
    "show cdp neighbors",                      // Connected Cisco devices
    "show lldp neighbors",                     // Connected devices (standard protocol)
    "show vlan brief",                         // VLAN configuration
    "show interfaces trunk",                   // Trunk port details
    "show spanning-tree summary"               // STP overview
};

SSHConnection::SSHConnection() : m_session(nullptr), m_socket(-1), m_connected(false) {
    // Initialize Winsock
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 0), &wsadata);
    
    // Initialize libssh2
    libssh2_init(0);
}

SSHConnection::~SSHConnection() {
    disconnect();
    libssh2_exit();
    WSACleanup();
}

bool SSHConnection::connect(const std::string& hostname, const std::string& username, 
                           const std::string& password, int port) {
    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Connect to host
    struct sockaddr_in sin; // "socket in"
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(hostname.c_str());
    
    if (::connect(m_socket, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
        std::cout << "Failed to connect to " << hostname << ":" << port << std::endl;
        closesocket(m_socket);
        return false;
    }
    
    // Create session
    m_session = libssh2_session_init();
    if (!m_session) {
        std::cout << "Failed to create SSH session" << std::endl;
        closesocket(m_socket);
        return false;
    }
    
    // Handshake
    if (libssh2_session_handshake(m_session, m_socket) != 0) {
        std::cout << "SSH handshake failed" << std::endl;
        libssh2_session_free(m_session);
        closesocket(m_socket);
        return false;
    }
    
    // Authenticate
    if (libssh2_userauth_password(m_session, username.c_str(), password.c_str()) != 0) {
        std::cout << "Authentication failed" << std::endl;
        libssh2_session_disconnect(m_session, "Authentication failed");
        libssh2_session_free(m_session);
        closesocket(m_socket);
        return false;
    }
    
    m_connected = true;
    std::cout << "Connected to " << hostname << " as " << username << std::endl;
    return true;
}

std::string SSHConnection::execute(const std::string& command) {
    if (!m_connected) {
        return "Error: Not connected";
    }
    
    // Open channel
    LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(m_session);
    if (!channel) {
        return "Error: Failed to open channel";
    }
    
    // Execute command
    if (libssh2_channel_exec(channel, command.c_str()) != 0) {
        libssh2_channel_free(channel);
        return "Error: Failed to execute command";
    }
    
    // Read output
    std::string result;
    char buffer[256];
    int bytesRead = 0;
    
    while ((bytesRead = libssh2_channel_read(channel, buffer, sizeof(buffer)-1)) > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }
    
    // Close and wait for remote to acknowledge
    libssh2_channel_close(channel);
    libssh2_channel_free(channel);
    

    return result;
}

void SSHConnection::executeMultipleCommands(const std::vector<std::string>& commands) {
    if (!m_connected) {
        std::cout << "Error: Not connected" << std::endl;
        return;
    }
    
    // Open a channel
    LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(m_session);
    if (!channel) {
        std::cout << "Error: Failed to open channel" << std::endl;
        return;
    }
    
    // Request a shell
    if (libssh2_channel_shell(channel) != 0) {
        std::cout << "Error: Failed to request shell" << std::endl;
        libssh2_channel_free(channel);
        return;
    }
    
    // Set non-blocking mode for reading
    libssh2_channel_set_blocking(channel, 0);
    
    // Wait for initial prompt and discard it
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    char buffer[4096];
    while (libssh2_channel_read(channel, buffer, sizeof(buffer)-1) > 0) {
        // Discard initial prompt/banner
    }
    
    // Execute each command
    for (const auto& command : commands) {
        std::string cmd = command + "\n";
        libssh2_channel_write(channel, cmd.c_str(), cmd.length());

        // Read until we haven't received data for a short period
        std::string output;
        int bytesRead;
        int emptyReads = 0;
        const int MAX_EMPTY_READS = 10;  // 10 * 50ms = 500ms total timeout
        
        while (emptyReads < MAX_EMPTY_READS) {
            bytesRead = libssh2_channel_read(channel, buffer, sizeof(buffer)-1);
            
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                output += buffer;
                emptyReads = 0;  // Reset counter when we get data
                
                // Check if we've received a prompt (ends with > or #)
                if (output.length() > 2) {
                    size_t lastNewline = output.find_last_of('\n');
                    if (lastNewline != std::string::npos) {
                        std::string lastLine = output.substr(lastNewline + 1);
                        if (!lastLine.empty() && 
                            (lastLine.back() == '>' || lastLine.back() == '#')) {
                            break;  // Found prompt, command complete
                        }
                    }
                }
            } else if (bytesRead == LIBSSH2_ERROR_EAGAIN) {
                // No data available yet
                emptyReads++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } else {
                // Error or channel closed
                break;
            }
        }
        
        std::cout << output << std::endl;
    }
    
    // Close channel
    libssh2_channel_close(channel);
    libssh2_channel_free(channel);
}

void SSHConnection::disconnect() {
    if (m_connected && m_session) {
        libssh2_session_disconnect(m_session, "Normal disconnect");
        libssh2_session_free(m_session);
        closesocket(m_socket);
        m_connected = false;
        std::cout << "Disconnected" << std::endl;
    }
}

bool SSHConnection::handleCommand(const std::vector<std::string>& arguments) {
    if (arguments.size() < 3) {
        std::cout << "Usage: ssh <username> <hostname> <password>" << std::endl;
        return true;
    }
    
    std::string username = arguments[0];
    std::string hostname = arguments[1];
    std::string password = arguments[2];
    
    if (connect(hostname, username, password)) {
        // Execute all discovery commands using shell mode
        executeMultipleCommands(DISCOVERY_COMMANDS);
        disconnect();
    }
    
    return true;

}
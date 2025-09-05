#include "SSHConnection.h"
#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <libssh2.h>

#pragma comment(lib, "ws2_32.lib")

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
    struct sockaddr_in sin;
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
    
    libssh2_channel_free(channel);
    return result;
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
        std::string result = execute("hostname");
        std::cout << "Hostname: " << result << std::endl;
        disconnect();
    }
    
    return true;
}
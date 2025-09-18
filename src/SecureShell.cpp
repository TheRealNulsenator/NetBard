#include "SecureShell.h"
#include "InputHandler.h"
#include <iostream>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <libssh2.h>
#include <thread>
#include <chrono>

const std::vector<std::string> SecureShell::DISCOVERY_COMMANDS = {
    "terminal length 0",                       // Return full output of all subsequent commands without dialogue
    "show interface status",                   // Basic interface configurations
    "show cdp neighbors",                      // Connected Cisco devices
};

const std::vector<char> SecureShell::PROMPT_ENDINGS = {'>', '#', '$', '%'};

SecureShell::SecureShell() : m_session(nullptr), m_socket(-1), m_connected(false) {
    libssh2_init(0);    // Initialize libssh2 
}

SecureShell::~SecureShell() {
    disconnect();
    libssh2_exit();
}

void SecureShell::handleCommand(const std::vector<std::string>& arguments) {
    if (arguments.size() < 3) {
        std::cout << "Usage: ssh <hostname> <username> <password>" << std::endl;
        return;
    }
    
    std::string hostname = arguments[0];
    std::string username = arguments[1];
    std::string password = arguments[2];
    
    if (connect(hostname, username, password)) {    // Execute all discovery commands using shell mode
        interactShell();
        disconnect();
    }

}

bool SecureShell::connect(const std::string& hostname, const std::string& username, 
                           const std::string& password, int port) {
    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in sin; // "socket in"
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(hostname.c_str());

    // Connect to host via socket
    if (::connect(m_socket, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
        std::cout << "Failed to connect to " << hostname << ":" << port << std::endl;
        closesocket(m_socket);
        return false;
    }
    // Create ssh session on opened socket connection
    m_session = libssh2_session_init();
    if (!m_session) {
        std::cout << "Failed to create SSH session" << std::endl;
        closesocket(m_socket);
        return false;
    }
    // Handshake with host to establish stateful connection
    if (libssh2_session_handshake(m_session, m_socket) != 0) {
        std::cout << "SSH handshake failed" << std::endl;
        libssh2_session_free(m_session);
        closesocket(m_socket);
        return false;
    } 
    // Authenticate! will allow most crypto algos, which windows ssh.exe does not by default, forcing me to do allll of this
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

std::string SecureShell::execute(const std::string& command) {
    if (!m_connected) {
        return "Error: Not connected";
    }
    LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(m_session);
    if (!channel) {// Open channel
        return "Error: Failed to open channel";
    }
    if (libssh2_channel_exec(channel, command.c_str()) != 0) {// Execute command
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

void SecureShell::interactShell() {
    if (!m_connected) {
        std::cout << "Error: Not connected" << std::endl;
    }
    LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(m_session);
    if (!channel) {    // Open a channel
        std::cout << "Error: Failed to open channel" << std::endl;
    }
    if (libssh2_channel_shell(channel) != 0) {    // Request a shell
        std::cout << "Error: Failed to request shell" << std::endl;
        libssh2_channel_free(channel);
    }
    
    //FIRST we send all default commands
    libssh2_channel_set_blocking(channel, 0);    // Set non-blocking mode for reading
    char buffer[4096];  //we will use this buffer a lot in the loop below
    std::string output;
    std::string cmd;
    std::cout << waitShellPrompt(channel, buffer);    // Wait for initial prompt
    for (const auto& command : DISCOVERY_COMMANDS) {    // Execute each command
        cmd = command + "\n";
        libssh2_channel_write(channel, cmd.c_str(), cmd.length());
        std::cout << waitShellPrompt(channel, buffer);  //await for return value   
    }
    //SECOND handle interactive commands
    InputHandler& inputHandler = InputHandler::getInstance();
    while(!libssh2_channel_eof(channel)){
        if (inputHandler.hasCommand()) {
            cmd = inputHandler.getCommand() + "\n";
            libssh2_channel_write(channel, cmd.c_str(), cmd.length());
            std::cout << waitShellPrompt(channel, buffer);  //await for return value   
        }
    }

    // Close channel
    libssh2_channel_close(channel);
    libssh2_channel_free(channel);
}

std::string SecureShell::waitShellPrompt(LIBSSH2_CHANNEL* channel, char* buffer){

    std::string output;
    int bytesRead;
    int emptyReads = 0;
    const int MAX_EMPTY_READS = 25; 
    const int CHECK_INTERVAL_MS = 50;
    
    while (emptyReads < MAX_EMPTY_READS) {    // Read until we haven't received data for a short period
        bytesRead = libssh2_channel_read(channel, buffer, sizeof(buffer)-1);
        
        if (bytesRead == LIBSSH2_ERROR_EAGAIN) {    // No data available yet
            emptyReads++;
            std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_INTERVAL_MS));
            continue;
        }
        
        if (bytesRead <= 0) {   // Error or channel closed
            break;
        }
        
        buffer[bytesRead] = '\0';
        output += buffer;
        emptyReads = 0;  // Reset counter when we get data

        if (output.length() <= 2) continue; // Check if we've received a prompt
        
        size_t lastNewline = output.find_last_of('\n');
        if (lastNewline == std::string::npos) continue;
        
        std::string lastLine = output.substr(lastNewline + 1);
        if (lastLine.empty()) continue;
        
        char lastChar = lastLine.back();
        for (char promptChar : PROMPT_ENDINGS) { // Check if last character matches any prompt ending
            if (lastChar == promptChar) {
                return output;  // Found prompt, command complete
            }
        }
    }
 
    return output;
}

void SecureShell::disconnect() {
    if (m_connected && m_session) {
        libssh2_session_disconnect(m_session, "Normal disconnect");
        libssh2_session_free(m_session);
        closesocket(m_socket);
        m_connected = false;
        std::cout << "Disconnected" << std::endl;
    }
}


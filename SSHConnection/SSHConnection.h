#ifndef SSH_CONNECTION_H
#define SSH_CONNECTION_H

#include <string>
#include <vector>

// Forward declarations
typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;
typedef struct _LIBSSH2_CHANNEL LIBSSH2_CHANNEL;

class SSHConnection {
public:
    SSHConnection();
    ~SSHConnection();
    
    // Simple connect and execute
    bool connect(const std::string& hostname, const std::string& username, const std::string& password, int port = 22);
    std::string execute(const std::string& command);
    void executeShell(const std::vector<std::string>& commands);
    std::string waitShellPrompt(LIBSSH2_CHANNEL* channel, char* buffer);
    void disconnect();
    
    // Command handler for CommandDispatcher
    bool handleCommand(const std::vector<std::string>& arguments);
    
private:
    LIBSSH2_SESSION* m_session;
    int m_socket;
    bool m_connected;
    
    static const std::vector<std::string> DISCOVERY_COMMANDS;
};

#endif // SSH_CONNECTION_H
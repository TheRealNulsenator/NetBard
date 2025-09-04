#ifndef SSH_CONNECTION_H
#define SSH_CONNECTION_H

#include <string>
#include <vector>

class SSHConnection {
public:
    SSHConnection();
    ~SSHConnection();
    
    // Test connection to a host
    bool testConnection(const std::string& hostname, const std::string& username, int port = 22);
    
    // Execute a command on the host
    std::string executeCommand(const std::string& hostname, const std::string& username, 
                              const std::string& command, int port = 22);
    
    // Command handler for CommandDispatcher
    bool handleCommand(const std::vector<std::string>& arguments);
    
private:
    std::string runSSHCommand(const std::string& sshCommand);
};

#endif // SSH_CONNECTION_H
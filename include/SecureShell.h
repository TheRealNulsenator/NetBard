#ifndef SECURE_SHELL_H
#define SECURE_SHELL_H

#include "vToolCommand.h"
#include <string>
#include <vector>

// Forward declarations
typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;
typedef struct _LIBSSH2_CHANNEL LIBSSH2_CHANNEL;

class SecureShell : public vToolCommand<SecureShell> {
public:
    // Static command metadata for CRTP base class
    static constexpr const char* COMMAND_PHRASE = "ssh";
    static constexpr const char* COMMAND_TIP = "Autorun ssh commands against a host. Usage: ssh <IP> <user> <pw>";
    
    ~SecureShell();

    // Command handler for CommandDispatcher
    void handleCommand(const std::vector<std::string>& arguments) override;
    
private:
    LIBSSH2_SESSION* m_session;
    int m_socket;
    bool m_connected;
    
    static const std::vector<std::string> DISCOVERY_COMMANDS;
    static const std::vector<char> PROMPT_ENDINGS;

    // Simple connect and execute
    bool connect(const std::string& hostname, const std::string& username, const std::string& password, int port = 22);
    std::string execute(const std::string& command);
    void interactShell();
    std::string waitShellPrompt(LIBSSH2_CHANNEL* channel, char* buffer);
    void disconnect();

    SecureShell();
    friend class vToolCommand<SecureShell>; //needed to allow getInstance to work in parent class
};

#endif // SECURE_SHELL_H
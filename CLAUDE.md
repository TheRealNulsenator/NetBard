# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Build the project using VSCode's build task:
- **VSCode**: Press `Ctrl+Shift+B` to build
- **Command line**:
```bash
g++ -g -I./include -IC:/msys64/mingw64/include main.cpp src/CommandDispatcher.cpp src/InputHandler.cpp src/SecureShell.cpp src/SubnetScanner.cpp src/LogStreambuf.cpp -lssh2 -lws2_32 -liphlpapi -o cartographer.exe
```

## Running the Application

```bash
./cartographer.exe
```

## High-Level Architecture

### Core Design Pattern: vToolCommand CRTP Template
The codebase uses a Curiously Recurring Template Pattern (CRTP) for tool commands. This provides:
- Singleton pattern enforcement via `getInstance()`
- Automatic command registration with CommandDispatcher on first access
- Two-phase command execution: `validateInput()` then `handleCommand()`
- Input validation prevents log file creation for invalid commands
- Integrated per-command logging to `logs/YYYYMMDD/command_details_HHMMSS.txt`
- Compile-time polymorphism with runtime dispatch

### Component Lifecycle & Dependencies
1. **main.cpp** initializes Winsock once for the entire program (WSAStartup/WSACleanup)
2. **InputHandler** singleton starts a detached thread that runs for process lifetime, collecting user input into a thread-safe queue
3. **CommandDispatcher** static class manages all command routing - initialized once, then accessed statically
4. Tool commands (SecureShell, PingScanner, TCPScanner) auto-register when their `getInstance()` is called

### Threading Architecture
- **InputHandler**: Dedicated input thread with mutex-protected command queue
- **PingScanner**: Uses work-stealing thread pool pattern (100 threads, atomic work distribution)
- **SecureShell**: Interactive shell mode with non-blocking reads and InputHandler integration

### Key Architectural Decisions
- No unnecessary abstraction layers - direct, focused implementations
- Static class pattern for CommandDispatcher (no instances needed)
- Singleton pattern for components that must have single instance (InputHandler, tool commands)
- All Winsock initialization centralized in main.cpp
- Logging integrated into vToolCommand base class, not scattered through implementations

## Available Commands

The application provides these built-in commands (case-insensitive):
- `help` - List all registered commands
- `quit`/`exit` - Exit the program
- `ssh <host> <user> <pass>` - Open interactive SSH session
- `ping <cidr>` - Ping sweep subnet for active hosts (e.g., `ping 192.168.1.0/24`)
- `tcp <ip>` - Scan TCP ports on target (e.g., `tcp 192.168.1.100`)

## Code Conventions

### Critical Rules
- **Windows headers**: Always include `<winsock2.h>` BEFORE `<windows.h>` to avoid conflicts
- **Pass by const reference**: Use `const std::string&` and `const std::vector<T>&` for parameters to avoid expensive copies
- **No magic numbers**: Always use named constants
- **Guard clauses**: Use early returns to flatten nested code
- **Thread safety**: Create handles/resources inside thread lambdas, not outside

### File Organization
- Headers in `include/` with traditional include guards (not #pragma once)
- Implementations in `src/`
- Each class gets its own header/implementation pair
- Include only what's directly needed in headers

## Development Notes

- The codebase uses libssh2 for SSH connections (not Windows ssh.exe) to support legacy devices
- Ping functionality uses Windows ICMP API with IcmpSendEcho
- All logging happens automatically via LogStreambuf when commands execute
- The application maintains three documentation files that should be updated during development:
  - `knowledge.md`: LLM working context
  - `code-style.md`: Observed coding patterns
  - `project-log.md`: Development history and decisions
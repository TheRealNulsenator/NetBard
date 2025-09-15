# Cartographer Project - Claude Context

## Quick Context
**Path:** C:\Users\nulsenb\OneDrive - Grantek\Documents\cartographer  
**Type:** Windows C++ application (main.cpp exists)  
**Git:** Not initialized  
**User:** Working on Grantek OneDrive  

## Documentation Responsibilities
**IMPORTANT: Update these files continuously throughout our work:**
- **knowledge.md** (this file): My working memory and governing document
  - Update silently with every relevant change
  - Track context, patterns, user preferences
  - Instructions for how I should operate
- **code-style.md**: Document user's coding preferences as I observe them
  - Update when user demonstrates new patterns
  - Track refactoring approaches, naming conventions, style choices
- **project-log.md**: Track work like reporting to a boss
  - Document time spent on issues
  - Record decisions made and rationale
  - Note large improvements and accomplishments
  - Include troubleshooting journeys and solutions

## What I Know About This Project
- Entry point: main.cpp
- Purpose: Network Cartographer - network mapping/visualization tool
- Build system: VSCode tasks.json, Ctrl+Shift+B to build
- Build command: g++ compiling all .cpp files to cartographer.exe
- Dependencies: Standard C++ libs only (thread, mutex, queue, atomic)
- User: Brad Nulsen at Grantek
- Input: Command-based system with threaded input handler

## Code Understanding
### File Structure
```
cartographer/
├── main.cpp (entry point at root level)
├── include/
│   ├── CommandDispatcher.h
│   ├── InputHandler.h
│   ├── LoggingStreambuf.h
│   ├── SecureShell.h
│   └── SubnetScanner.h
├── src/
│   ├── CommandDispatcher.cpp
│   ├── InputHandler.cpp
│   ├── LoggingStreambuf.cpp
│   ├── SecureShell.cpp
│   └── SubnetScanner.cpp
├── knowledge.md (this file - LLM context)
├── code-style.md (C++ style guide)
└── project-log.md (human-readable project decisions/history)
```

### Architecture
- **main.cpp**: Creates all components, registers command handlers, runs main loop
  - **Initializes Winsock once for entire program** (WSAStartup/WSACleanup)
  - Gets singleton instances of InputHandler and CommandDispatcher
  - Gets singleton instances of SecureShell and SubnetScanner (both via getInstance())
  - Auto-registration happens on getInstance() calls for tool commands
- **vToolCommand**: CRTP base template for tool commands
  - Provides singleton pattern via getInstance()
  - Auto-registers commands with CommandDispatcher on first use
  - Uses static constexpr for command metadata (COMMAND_PHRASE, COMMAND_TIP)
  - Virtual handleCommand() for runtime polymorphism
  - Enforces design contract at compile time via template instantiation
  - Protected save_results() method for file output (bare bones implementation)
- **CommandDispatcher**: Static class dispatcher with built-in help/quit/exit
  - Static class pattern: All methods and data are static, no instances
  - Initialize with `CommandDispatcher::initialize()`
  - Access methods directly: `CommandDispatcher::processCommand()`, `CommandDispatcher::registerCommand()`
  - All commands use uniform interface: `bool(const std::vector<std::string>&)`
  - Splits input into words, removes command name, passes arguments to handlers
  - Self-registers help/quit/exit as fundamental commands
  - Help command auto-lists all registered commands with tips
  - **Logging integration**: Initializes LoggingStreambuf and redirects cout
    - Currently logs ALL commands including built-ins (unintended)
    - TODO: Move logging to vToolCommand for selective logging
- **LoggingStreambuf**: Standalone logging class that inherits from std::streambuf
  - Duplicates cout output to timestamped log files
  - Owns file lifecycle via unique_ptr<ofstream>
  - Creates directory structure: `logs/YYYYMMDD/command_HHMMSS.txt`
  - Auto-creates directories using std::filesystem
  - Real-time file flushing for immediate output
  - Thread-safe design for future multi-command scenarios
  - TODO: Instantiate per vToolCommand instead of globally
- **InputHandler**: Singleton thread-safe command input system (queue-based)
  - Singleton pattern: Lazy initialization, thread-safe since C++11
  - Access via `InputHandler::getInstance()`
  - Private constructor, deleted copy constructor/assignment operator
- **SecureShell** (formerly SSHConnection): Barebones libssh2 implementation
  - Direct libssh2 API usage (no ssh.exe)
  - Password authentication only
  - Simple connect/execute/disconnect model
  - Inherits from vToolCommand<SecureShell> for singleton pattern
  - Uses static constexpr for command metadata (COMMAND_PHRASE, COMMAND_TIP)
  - Has handleCommand() method for CommandDispatcher integration
  - Requires: libssh2 library
  - **Important**: Proper channel cleanup required between commands (send_eof, wait_eof, wait_closed)
- **SubnetScanner**: Network subnet scanning and host discovery
  - Inherits from vToolCommand<SubnetScanner> for singleton pattern
  - Uses static constexpr for command metadata (COMMAND_PHRASE = "scan")
  - Handles CIDR notation parsing (supports both / and \ delimiters)
  - Converts IP addresses to binary representation (uint32_t)
  - Creates subnet masks from CIDR notation
  - Generates address ranges for scanning
  - Refactored architecture: Separated CIDR parsing from scanning logic
  - **Ping implementation**: Windows ICMP API (work-stealing thread pool)
    - Uses IcmpSendEcho for host discovery
    - **Threading model**: Work-stealing thread pool pattern
      - Fixed pool of 100 threads created upfront
      - Atomic index for work distribution (`fetch_add` for thread safety)
      - Each thread pulls work until queue empty
      - ICMP handle created inside each thread (avoids reference capture issue)
    - Retry logic: 3 attempts per host, 500ms timeout each
    - Mutex protection for console output and Host_Statuses map
    - Uses emplace_back to avoid thread copy construction
    - 5ms delay between thread spawns (reduced from 25ms)
  - **Data structures**:
    - `Host_Addresses`: Vector of all IPs to scan
    - `Host_Statuses`: Map tracking ping results (address -> bool)
    - Public members for network info access
  - Has handleCommand() method for CommandDispatcher integration
  - Command: `scan <cidr>` (e.g., scan 192.168.1.0/24)
  - Requires linking: -lws2_32 -liphlpapi
- User preference: No unnecessary abstraction layers (rejected Application class)

### Code Style
- **Refer to code-style.md for all styling decisions**
- User wants no magic numbers - always use named constants
- Simple, widely-accepted C++ conventions

### Key Code Patterns
- **Guard clauses**: Use early returns/continue to flatten nested code
- **Refactoring for clarity**: Extract wait/prompt logic to separate functions
- **Buffer reuse**: Pass buffers to avoid repeated allocations
- **Function naming**: User renamed executeMultipleCommands to executeShell (more accurate)
- **Constants**: User adds descriptive constants like CHECK_INTERVAL_MS with comments
- **Code organization**: Implementation details go in helper functions, main flow stays clean
- **Magic value elimination**: Replace hardcoded chars with named constant arrays (PROMPT_ENDINGS)
- **Scalable patterns**: Use loops/arrays instead of hardcoded conditionals for extensibility

### Important Functions/Classes
- **InputHandler**: Fire-and-forget detached thread, runs for process lifetime
  - Singleton instance accessed via `getInstance()`
  - No explicit lifecycle management needed
  - Leverages OS process cleanup
  - Private constructor prevents direct instantiation
- **CommandDispatcher**: Central command routing system
  - Static class - all methods accessed directly (no instances)
  - Call `initialize()` once at startup
  - Manages command registration and execution

## User Preferences & Patterns
- **KISS principle** - Don't add code for future scenarios
- No premature abstraction or "future enhancement" comments
- **Documentation discipline**: Expects continuous updates to markdown files
- **Refactoring style**: Extract complex logic, use guard clauses, flatten nesting
- **Function naming**: Prefers accurate names (executeShell vs executeMultipleCommands)
- **Code reviews**: Will call out when documentation isn't being maintained

## Technical Considerations
- Windows ssh.exe has crypto restrictions that affect legacy device connections
- May need libssh2 for better legacy SSH support without system modifications
- User wants concise responses
- User wants me to maintain context via this file
- Working environment: Windows, OneDrive synced
- Update knowledge.md silently with every prompt when relevant
- Keep information density high - no fluff
- Don't create noise about updates
- Prefers organized folder structure (classes in own folders)
- Makes own code formatting adjustments - observe and adapt
- **Code style**: Self-documenting, verbose variable names over brevity
- Prefers code that "flows naturally" - readable over dense
- **IMPORTANT**: User does NOT want test builds - just write the code correctly

## User Background & Role
- Industrial systems software engineer
- Bachelor's in Computer Engineering
- Strong CS theory & hardware architecture knowledge
- Experienced with engineering solutions
- **My Role**: C++ mentor/guide - help with syntax, idioms, best practices
- **NOT**: Don't architect or over-engineer - user handles design decisions
- Teach C++ patterns as senior engineer would

## Technical Context
### Build Commands
*Not yet discovered*

### Test Commands
*Not yet discovered*

### Common Operations
*Will document user's typical requests*

## Previous Conversations Summary
### Session 1 (2025-09-04)
- User requested creation of knowledge.md for context persistence
- User clarified this is for MY context, not documentation

## Ongoing Work
- 🔧 Multi-Session SSH Architecture (2025-09-12)
  - **Decision**: Implement concurrent SSH sessions with session switching
  - **Goal**: Network scanning while maintaining persistent SSH connections
  - **Architecture**: SessionManager with per-session threads and queues
  - **Commands**: sessions, switch <id>, kill <id>, Ctrl+Z to detach
  - **Implementation phases**:
    1. Single SSH session with thread separation
    2. Session manager with multiple sessions
    3. Background scanning while in session
    4. Session switching and management commands
  - **Key components**:
    - SessionManager class to track all sessions
    - Per-session input/output queues
    - Active session routing in main loop
    - Session-specific logging
- 🔧 Per-command logging system (2025-09-11)
  - Designed TeeStreambuf architecture in CommandDispatcher
  - Keeps all logging code contained in single class
  - Header updated with infrastructure, implementation pending
  - User will complete implementation of startLogging()/stopLogging()
- ✅ Winsock initialization refactor (2025-09-08)
  - Moved WSAStartup/WSACleanup from individual components to main.cpp
  - Single initialization for entire program (best practice)
  - Removed from SecureShell and SubnetScanner
- ✅ vToolCommand updates (2025-09-08)
  - Refactored to use static constexpr for command metadata
  - Added save_results() protected method for file output
  - Both SecureShell and SubnetScanner now inherit from vToolCommand
- ✅ SSH shell mode successfully implemented (2025-01-06)
  - Fixed "Failed to open channel" error by using shell mode
  - Refactored waitShellPrompt() using guard clauses
  - Added automated discovery commands for network devices
- ✅ SubnetScanner implementation (2025-09-05 - 2025-09-06)
  - CIDR notation parsing with flexible delimiters
  - Binary IP address conversion and manipulation
  - Subnet mask generation from CIDR notation
  - Address range calculation for host discovery
  - Ping functionality using Windows ICMP API
  - Multithreaded scanning implementation
  - Fixed memory leak in ping retry logic
  - Fixed ICMP handle thread safety issues
- ✅ Work-stealing thread pool implementation (2025-09-06)
  - Refactored from one-thread-per-IP to fixed thread pool
  - Implemented atomic work distribution
  - Fixed race conditions and output bugs
- ✅ Singleton pattern implementation (2025-09-07)
  - Converted InputHandler and CommandDispatcher to singletons
  - Using lazy initialization pattern (thread-safe since C++11)
  - Prevents multiple instances and improves global access

## Critical Notes
- **Must maintain documentation**: User explicitly requires updating knowledge.md, code-style.md, and project-log.md throughout work
- **SSH device limitation**: Some network devices only allow one channel per session - use shell mode for multiple commands

## Cleanup/Polish Task Checklist
**Trigger phrase**: "cleanup", "polish", or similar
**Purpose**: Systematic code quality improvements without changing functionality

### Always Check:
1. **Header Guards**
   - Ensure all headers use proper include guards matching their filename
   - Format: `HEADER_NAME_H` (e.g., `SUBNET_SCANNER_H` for SubnetScanner.h)
   - Never use pragma once (prefer traditional guards)

2. **Include Optimization**
   - Remove duplicate includes between .h and .cpp files
   - Remove unused includes (especially `<thread>`, `<mutex>` if not used)
   - Add required includes for types (e.g., `<cstdint>` for uint32_t)
   - Header files should only include what they directly need
   - Implementation files should not re-include what's in their header
   - **Windows headers**: ALWAYS include `<winsock2.h>` before `<windows.h>` to avoid conflicts

3. **Naming Consistency**
   - Private member variables: check for m_ prefix or snake_case consistency
   - Function names: camelCase (some use snake_case - standardize)
   - Constants: UPPER_SNAKE_CASE
   - Fix any typos (e.g., subnet_cider → subnet_cidr)

4. **Code Organization**
   - Ensure consistent spacing and indentation (4 spaces)
   - Remove unnecessary blank lines
   - Add blank lines between logical sections
   - Ensure consistent brace style

5. **Comments & Documentation**
   - Remove or update outdated comments
   - Ensure inline comments are meaningful
   - Update documentation files if behavior changed

6. **Const Correctness**
   - Mark methods const where appropriate
   - Use const parameters where values shouldn't change
   - Check return types for unnecessary const on primitives
   - **Pass by const reference**: Convert `const std::string` and `const std::vector` parameters to `const&` to avoid copies

7. **Error Messages**
   - Ensure all error messages are clear and consistent
   - Use consistent capitalization in output messages

8. **Magic Numbers**
   - Replace any remaining magic numbers with named constants
   - Document any non-obvious constant values

9. **Parameter Optimization**
   - Pass objects (string, vector, custom classes) by const reference instead of value
   - Keep primitives (int, uint32_t, bool, char) as pass-by-value
   - Check for unnecessary copying in function parameters

### Quick Checks:
- Run build to ensure no warnings
- Check for TODO/FIXME comments that can be addressed
- Verify all files end with newline
- Check for trailing whitespace

---
*Last Updated: 2025-09-06 - This file preserves Claude's working context for the Cartographer project*
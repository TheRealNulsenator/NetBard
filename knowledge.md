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
│   ├── SSHConnection.h
│   └── SubnetScanner.h
├── src/
│   ├── CommandDispatcher.cpp
│   ├── InputHandler.cpp
│   ├── SSHConnection.cpp
│   └── SubnetScanner.cpp
├── knowledge.md (this file - LLM context)
├── code-style.md (C++ style guide)
└── project-log.md (human-readable project decisions/history)
```

### Architecture
- **main.cpp**: Creates all components, registers command handlers, runs main loop
  - Owns SSHConnection, InputHandler, CommandDispatcher, SubnetScanner
  - Registers commands using std::bind for cleaner code
- **CommandDispatcher**: Dispatcher with built-in help/quit/exit
  - All commands use uniform interface: `bool(const std::vector<std::string>&)`
  - Splits input into words, removes command name, passes arguments to handlers
  - Self-registers help/quit/exit as fundamental commands
  - Help command auto-lists all registered commands with tips
- **InputHandler**: Thread-safe command input system (queue-based)
- **SSHConnection**: Barebones libssh2 implementation
  - Direct libssh2 API usage (no ssh.exe)
  - Password authentication only
  - Simple connect/execute/disconnect model
  - Has handleCommand() method for CommandDispatcher integration
  - Requires: libssh2 library
  - **Important**: Proper channel cleanup required between commands (send_eof, wait_eof, wait_closed)
- **SubnetScanner**: Network subnet scanning and host discovery
  - Handles CIDR notation parsing (supports both / and \ delimiters)
  - Converts IP addresses to binary representation (uint32_t)
  - Creates subnet masks from CIDR notation
  - Generates address ranges for scanning
  - **Ping implementation**: Windows ICMP API
    - Uses IcmpSendEcho for host discovery
    - Optimized: WSAStartup called once per scan
    - Optimized: ICMP handle reused across all pings
    - 1 second timeout per host
    - Stores alive hosts in subnet_hosts vector
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
  - No explicit lifecycle management needed
  - Leverages OS process cleanup

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
  - Performance optimizations for batch scanning

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

7. **Error Messages**
   - Ensure all error messages are clear and consistent
   - Use consistent capitalization in output messages

8. **Magic Numbers**
   - Replace any remaining magic numbers with named constants
   - Document any non-obvious constant values

### Quick Checks:
- Run build to ensure no warnings
- Check for TODO/FIXME comments that can be addressed
- Verify all files end with newline
- Check for trailing whitespace

---
*Last Updated: 2025-09-06 - This file preserves Claude's working context for the Cartographer project*
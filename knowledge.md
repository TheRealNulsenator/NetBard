# Cartographer Project - Claude Context

## Quick Context
**Path:** C:\Users\nulsenb\OneDrive - Grantek\Documents\cartographer  
**Type:** Windows C++ application (main.cpp exists)  
**Git:** Not initialized  
**User:** Working on Grantek OneDrive  

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
├── main.cpp (orchestrates program, no class/function definitions)
├── CommandDispatcher/
│   ├── CommandDispatcher.h
│   └── CommandDispatcher.cpp
├── InputHandler/
│   ├── InputHandler.h
│   └── InputHandler.cpp
├── knowledge.md (this file - LLM context)
├── code-style.md (C++ style guide)
└── project-log.md (human-readable project decisions/history)
```

### Architecture
- **main.cpp**: Creates all components, registers command handlers, runs main loop
  - Owns SSHConnection, InputHandler, CommandDispatcher
  - Registers commands using std::bind for cleaner code
- **CommandDispatcher**: Dispatcher with built-in help/quit/exit
  - All commands use uniform interface: `bool(const std::vector<std::string>&)`
  - Splits input into words, passes to handlers
  - Self-registers help/quit/exit as fundamental commands
  - Help command auto-lists all registered commands
- **InputHandler**: Thread-safe command input system (queue-based)
- **SSHConnection**: Simple SSH wrapper around ssh.exe
  - Stateless design (no connection management)
  - Uses _popen/_pclose to run ssh commands
  - Assumes SSH keys are configured
  - Has handleCommand() method for CommandDispatcher integration
- User preference: No unnecessary abstraction layers (rejected Application class)

### Code Style
- **Refer to code-style.md for all styling decisions**
- User wants no magic numbers - always use named constants
- Simple, widely-accepted C++ conventions

### Key Code Patterns
*Will document as I learn them*

### Important Functions/Classes
- **InputHandler**: Fire-and-forget detached thread, runs for process lifetime
  - No explicit lifecycle management needed
  - Leverages OS process cleanup

## User Preferences & Patterns
- **KISS principle** - Don't add code for future scenarios
- No premature abstraction or "future enhancement" comments
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
*Track incomplete tasks for resumption*

## Critical Notes
*Important warnings, gotchas, or user-specific requirements*

---
*Last Updated: 2025-09-04 - This file preserves Claude's working context for the Cartographer project*
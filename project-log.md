# Network Cartographer - Project Log

## Project Overview
**Purpose**: Network mapping and visualization tool  
**Author**: Brad Nulsen, Grantek  
**Started**: January 2025  

---

## Design Decisions

### 2025-01-04: Command Registration Architecture  
- **Decision**: Commands registered in main, not CommandDispatcher
- **Rationale**: Better separation of concerns
- **Benefits**:
  - CommandDispatcher is a pure dispatcher with no dependencies
  - Main owns all components and wires them together
  - Commands can capture local objects via lambda closures
  - More testable and flexible

### 2025-01-04: SSH Implementation Strategy
- **Initial approach**: Windows ssh.exe via _popen
- **Problem**: Windows SSH restrictions on legacy crypto algorithms
- **Final decision**: Direct libssh2 implementation
- **Rationale**: Full control over crypto, supports legacy devices
- **Implementation**: Barebones - just connect/execute/disconnect
- **Trade-offs**: 
  - Pros: Works with legacy devices, no system config needed
  - Cons: Requires libssh2 dependency

### 2025-01-04: Initial Architecture
- **Decision**: Console-based application with keyboard input handling
- **Rationale**: Start with simple CLI interface, can add GUI later if needed
- **Implementation**: Using Windows console I/O (conio.h) for non-blocking keyboard input


### 2025-01-04: InputHandler Simplification
- **Decision**: Fire-and-forget detached thread pattern
- **Rationale**: Input collection is always needed until process dies
- **Benefits**: No lifecycle management, no race conditions, leverages OS cleanup
- **Philosophy**: KISS - removed unnecessary complexity when process termination handles cleanup perfectly

---

## Technical Adjustments

### 2025-01-06: Directory Structure Reorganization
- **Decision**: Moved to traditional include/src folder structure
- **Rationale**: Better maintainability and industry standard organization
- **Changes**:
  - Headers moved to `include/`
  - Implementation files moved to `src/`
  - main.cpp kept at root (user preference)
  - Updated tasks.json to use new paths
- **Benefits**: Cleaner separation, easier to navigate, standard C++ project layout

### 2025-01-06: SSH Implementation Refinements
- **Time spent**: ~1 hour debugging and refactoring
- **Refactored waitShellPrompt()**: Applied guard clauses to reduce nesting from 4 to 2 levels
- **Improved function naming**: executeMultipleCommands → executeShell (user correction)
- **Added command list**: Centralized network discovery commands in static const vector
- **Code organization**: Extracted prompt waiting logic to separate function for clarity

### 2025-01-04: Code Quality Standards
- Established code style guide (code-style.md)
- Eliminated magic numbers - using named constants
- Adopted standard C++ naming conventions

---

## Complexity Points

### Console Input Handling
- **Challenge**: Need non-blocking command input system
- **Evolution**:
  1. conio.h (deprecated)
  2. Windows API (GetAsyncKeyState) - for ESC key
  3. Threaded input handler with command queue (current)
- **Final solution**: Separate InputHandler class with thread-safe queue
- **Rationale**: Decoupled, extensible, uses only standard C++ libs
- **Benefits**: Platform-independent, easy to extend, clean separation

---

## Architecture Evolution

### Phase 1: Foundation (Current)
- Basic console application structure
- Event loop with keyboard handling  
- Clean exit mechanism
- **2025-01-04**: Refactored to clean architecture
  - main.cpp orchestrates but defines no functions/classes
  - Application class created then removed (unnecessary abstraction)
  - CommandDispatcher for extensible command handling
  - InputHandler for thread-safe input
  - Each class has single responsibility
  - Classes organized into their own folders for better structure

### Phase 2: Core Functionality (Planned)
- Network discovery logic
- Data structures for network topology
- Connection management

### Phase 3: Visualization (Future)
- Display network map
- Interactive navigation
- Real-time updates

---

## Dependencies & Libraries

| Library | Purpose | Added | Notes |
|---------|---------|-------|-------|
| iostream | Standard I/O | 2025-01-04 | Core C++ |
| conio.h | Console input | 2025-01-04 | Windows-specific |

---

## Performance Considerations

### Input Loop
- Non-blocking design prevents CPU spinning
- Minimal overhead when idle
- Responsive to user input

---

## Security Considerations

*To be documented as security-relevant decisions are made*

---

## Testing Strategy

*To be established*

---

## Known Issues & Limitations

1. **Console limitations**: Text-only interface for now

## Resolved Issues

1. **Double-enter on quit** (2025-01-04): Fixed by simplifying InputHandler
   - Root cause: std::getline blocks, preventing clean thread join
   - Initial solution: Detach thread on stop()
   - Final solution: Fire-and-forget pattern - thread runs for process lifetime
   - Removed unnecessary start/stop lifecycle management

2. **SSH Channel Error on Multiple Commands** (2025-01-05): Fixed with shell mode
   - Symptom: "Failed to open channel" when executing second command
   - Root cause: Some devices limit one channel per SSH session
   - Initial attempt: Added proper channel cleanup (didn't work)
   - Final solution: Implemented shell mode - single channel for all commands
   - Added `executeShell()` for multi-command execution
   - Added `waitShellPrompt()` to detect command completion via prompt
   - Refactored prompt waiting to reduce nesting using guard clauses
   - Impact: Reliably executes multiple commands on restrictive devices

---

## Future Enhancements

- [ ] Cross-platform input handling
- [ ] Configuration file support
- [ ] Network protocol selection
- [ ] Export capabilities
- [ ] GUI version

---

## Meeting Notes & Decisions

*Document key discussions and decisions from team meetings*

---

*Last Updated: 2025-01-06*
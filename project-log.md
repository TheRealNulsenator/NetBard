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
- **Decision**: PIMPL + Abstract Factory pattern with pluggable backends
- **Rationale**: Maximum flexibility, runtime backend selection
- **Current backend**: WindowsOpenSSHImpl using ssh.exe
- **Architecture benefits**:
  - Abstract base defines interface contract
  - Factory method selects appropriate implementation
  - Easy to add new backends without changing public API
  - Enables mock implementations for testing
- **Trade-offs**: 
  - Pros: Extensible, testable, no lock-in to specific implementation
  - Cons: Slightly more complex than direct implementation
- **Future backends**: LibSSH2, PowerShell remoting, Mock for tests

### 2025-01-04: Initial Architecture
- **Decision**: Console-based application with keyboard input handling
- **Rationale**: Start with simple CLI interface, can add GUI later if needed
- **Implementation**: Using Windows console I/O (conio.h) for non-blocking keyboard input

### 2025-01-04: Exit Mechanism
- **Decision**: Use ESC key as program exit trigger
- **Rationale**: Standard convention, doesn't interfere with typical input
- **Alternative considered**: Ctrl+C (rejected - too abrupt, no cleanup opportunity)

### 2025-01-04: InputHandler Simplification
- **Decision**: Fire-and-forget detached thread pattern
- **Rationale**: Input collection is always needed until process dies
- **Benefits**: No lifecycle management, no race conditions, leverages OS cleanup
- **Philosophy**: KISS - removed unnecessary complexity when process termination handles cleanup perfectly

---

## Technical Adjustments

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

*Last Updated: 2025-01-04*
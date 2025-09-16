# Network Cartographer - Project Log

## Project Overview
**Purpose**: Network mapping and visualization tool  
**Author**: Brad Nulsen, Grantek  
**Started**: January 2025  

---

## Design Decisions

### 2025-09-05: Code Polish/Cleanup Process
- **Decision**: Established systematic cleanup checklist
- **Trigger**: User says "cleanup" or "polish"
- **Rationale**: Maintain consistent code quality without repeating instructions
- **Checklist includes**:
  - Header guard verification and naming consistency
  - Include optimization for faster compile times
  - Naming convention enforcement
  - Code organization and formatting
  - Comment and documentation updates
  - Const correctness review
  - Error message consistency
  - Magic number elimination
- **Benefits**: 
  - Faster compile times through include optimization
  - More maintainable code through consistency
  - Reduced technical debt through regular cleanup

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
- **2025-09-06 Update**: Renamed SSHConnection to SecureShell for clarity
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

### 2025-09-06: Major Code Cleanup & Optimization Refactor
- **Scope**: Full codebase polish following established cleanup checklist
- **Time spent**: ~30 minutes systematic review
- **Trigger**: User requested "cleanup" command execution
- **Major improvements**:
  
#### Include Optimization
- Removed duplicate `winsock2.h` from SubnetScanner.h (windows.h already includes it)
- Removed unused includes: `<iomanip>`, `<semaphore>`, duplicate `<cstdint>`
- Added missing `<chrono>` for std::chrono usage
- Result: Faster compile times, cleaner dependencies

#### Const Correctness Refactor
- Removed unnecessary `const` on return types for primitives (bool, vector)
- **Performance optimization**: Converted all object parameters from pass-by-value to pass-by-const-reference
  - Changed: `const std::string param` → `const std::string& param`
  - Changed: `const std::vector<std::string> param` → `const std::vector<std::string>& param`
  - Impact: Eliminates expensive string/vector copying on every function call
  - Scope: 4 functions in SubnetScanner (find_hosts, unwrap_cidr, address_to_bits, create_subnet_mask)

#### Magic Number Elimination
- Replaced all magic numbers with named constants:
  - `IPV4_OCTETS = 4`
  - `MIN_OCTET = 0`, `MAX_OCTET = 255`
  - `BITS_PER_OCTET = 8`
  - `EXPECTED_DOTS = 3`, `EXPECTED_TOKENS = 5`
  - `MIN_SUBNET_BITS = 0`, `MAX_SUBNET_BITS = 32`
  - `MAX_PING_ATTEMPTS = 4`
  - `MS_BETWEEN_THREAD_SPAWNS = 50`

#### Code Quality
- Fixed variable naming inconsistency (MS_BETWEEN_PINGS vs MS_BETWEEN_THREAD_SPAWNS)
- Standardized error message capitalization
- Removed extra blank lines and trailing whitespace
- Fixed missing newline at end of file

#### Documentation Updates
- **Enhanced cleanup checklist**: Added parameter optimization as standard check item
- **code-style.md**: Added comprehensive "Parameter Passing Guidelines" section
- **knowledge.md**: Updated cleanup checklist with pass-by-const-reference pattern

#### Impact
- **Performance**: Significant improvement in function call overhead, especially for subnet scanning operations
- **Maintainability**: All magic numbers now self-documenting
- **Compile time**: Reduced through include optimization
- **Code quality**: Consistent style and naming throughout

This refactor demonstrates the value of systematic cleanup processes and the importance of performance-aware coding patterns in C++.

### 2025-09-05 - 2025-09-06: SubnetScanner Implementation
- **Decision**: Implemented network subnet scanning component with ping functionality
- **Features**:
  - CIDR notation parsing with flexible delimiter support (/ or \)
  - Binary IP address conversion using bitwise operations
  - Subnet mask generation from CIDR notation
  - Address range calculation for host discovery
  - **Ping functionality**: Windows ICMP API integration
  - **Multithreaded scanning**: One thread per IP address

#### Initial Implementation (Single-threaded)
- **ICMP Ping Implementation**:
  - Uses Windows IcmpCreateFile/IcmpSendEcho API
  - 1 second timeout per host
  - Stores discovered hosts in subnet_hosts vector
- **Performance Optimizations** (user-driven):
  - WSAStartup/WSACleanup called once per scan (not per ping)
  - ICMP handle created once and reused for all pings
  - Result: Significantly faster subnet scanning

#### Multithreading Milestone (2025-09-06)
- **Threading Implementation**:
  - One thread spawned per IP address to scan
  - Each thread gets its own ICMP handle (thread safety)
  - 50ms delay between thread spawns (protect old PLCs from ping storms)
  - Uses `emplace_back` to avoid thread copy construction
- **Retry Logic Added**:
  - 4 attempts per host
  - 500ms timeout per attempt
  - Early exit on success
- **Thread Safety**:
  - Mutex protection for console output
  - Mutex protection for subnet_hosts vector (removed separate mutex, combined into single critical section)
- **Bug Fixes** (user-identified):
  1. **Memory leak**: Fixed - buffer now freed on both success and failure paths
  2. **ICMP handle sharing**: Fixed - each thread creates its own handle
  3. **Thread limit not enforced**: Identified - MAX_THREADS defined but not used (planned fix with semaphore)

- **Code patterns**:
  - Lambda captures in thread creation `[&, binary_ip]`
  - RAII pattern with `lock_guard` for mutex management
  - Resource cleanup in all code paths
  - Extensive use of const return types for safety
  - Early return validation pattern
  - Descriptive variable names (e.g., `valid_octet_count`, `valid_mask_count`)
  - Comments explaining edge cases and logic
  - Resource management: proper cleanup of Windows handles

### 2025-09-06: Class Naming Refactor
- **Change**: Renamed SSHConnection to SecureShell
- **Rationale**: More descriptive and professional naming
- **Scope**: 
  - Renamed files: SSHConnection.h/cpp → SecureShell.h/cpp
  - Updated all references in main.cpp
  - Updated tasks.json build configuration
  - Updated all documentation
- **Impact**: Better code clarity, no functional changes

### 2025-09-07: Singleton Pattern Implementation
- **Change**: Converted InputHandler to singleton pattern
- **Rationale**: 
  - Ensures single instance throughout application lifetime
  - Provides global access point for input handling
  - Prevents accidental multiple instantiation
- **Implementation**:
  - Lazy initialization pattern (thread-safe since C++11)
  - Static `getInstance()` method returns reference to static instance
  - Private constructors prevent direct instantiation
  - Deleted copy constructor and assignment operator
- **Code pattern**:
  ```cpp
  static ClassName& getInstance() {
      static ClassName instance;  // Lazy init, thread-safe
      return instance;
  }
  ```
- **Usage change in main.cpp**:
  - Before: `InputHandler inputHandler;`
  - After: `InputHandler& inputHandler = InputHandler::getInstance();`

### 2025-09-07: CommandDispatcher Static Class Refactor
- **Change**: Converted CommandDispatcher from singleton to static class pattern
- **Rationale**:
  - CommandDispatcher has no instance-specific state
  - All state is effectively global (command registry)
  - Static class pattern is cleaner for stateless utilities
  - Eliminates unnecessary instance management
- **Implementation**:
  - All methods and data members are static
  - Deleted all constructors to prevent instantiation
  - Added `initialize()` method for one-time setup
  - Static maps for commands and tips (s_commands, s_tips)
- **Usage change in main.cpp**:
  - Before: `CommandDispatcher& commandDispatcher = CommandDispatcher::getInstance();`
  - After: `CommandDispatcher::initialize();`
  - Method calls: `commandDispatcher.processCommand()` → `CommandDispatcher::processCommand()`
- **Benefits**:
  - Clearer intent - this is a utility class, not an object
  - No instance management overhead
  - Direct static method calls
  - Better matches actual usage pattern

#### Work-Stealing Thread Pool Implementation (2025-09-06)
- **User-driven design decision**: Implement work-stealing thread pool pattern
- **Motivation**: 
  - Previous approach spawned one thread per IP (inefficient)
  - Thread creation/destruction overhead
  - No actual thread limiting despite MAX_THREADS constant
- **Implementation**:
  - Fixed pool of 100 threads created upfront
  - Atomic index (`std::atomic<int>`) for work distribution
  - Threads use `fetch_add` to atomically get next work item
  - Each thread loops until work queue exhausted
  - ICMP handle created inside thread lambda (avoids reference capture issues)
- **Architecture changes**:
  - Separated CIDR parsing logic into `handleCommand`
  - `find_hosts()` now parameterless, works on member variables
  - Added public members: `Host_Addresses`, `Host_Statuses` map, network info
- **Bug fixes during implementation**:
  1. **Race condition**: Initial implementation read index then incremented (not atomic)
     - Fixed: Use `fetch_add` to atomically get-and-increment
  2. **Output bug**: Printed "is alive" for all addresses
     - Fixed: Only print if `pingable == true`
  3. **ICMP handle sharing**: Reference capture would share handle across threads
     - Fixed: Create handle inside lambda
  4. **Scan summary**: Showed total scanned, not alive count
     - Fixed: Count only alive hosts from map
- **Performance improvements**:
  - Thread spawn delay reduced from 25ms to 10ms (later to 5ms)
  - No per-IP thread overhead
  - Better CPU utilization (threads stay busy)
  - Load balancing (fast threads automatically take more work)

- **Cleanup performed**:
  - Fixed header guard from `SUBNET_PINGER_H` to `SUBNET_SCANNER_H`
  - Removed unused includes (`<thread>`, `<mutex>` from header - moved to cpp)
  - Removed duplicate include (`<string>` from cpp)
  - Added required includes (`<cstdint>` for uint32_t, Windows headers for ICMP)
  - Fixed typo: `subnet_cider` → `subnet_cidr`
  - Result: Cleaner headers, faster compile times

- **Build configuration updated**:
  - Added -liphlpapi to tasks.json for ICMP API functions
  - Already had -lws2_32 for Winsock

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

### Phase 2: Core Functionality (In Progress)
- Network discovery logic
  - **2025-09-05**: SubnetScanner component added
  - CIDR notation parsing and validation
  - IP address to binary conversion
  - Subnet mask generation
  - Address range calculation
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
| libssh2 | SSH connections | 2025-01-05 | For legacy device support |
| ws2_32 | Winsock | 2025-09-06 | Windows networking |
| iphlpapi | ICMP API | 2025-09-06 | Windows ping functionality |

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

### 2025-01-08: vToolCommand Virtual Base Class Pattern
- **Change**: Created template base class for tool commands with automatic singleton and registration
- **Motivation**: Consolidate common patterns across all tool commands
- **Design Goals**:
  - Single place to define self-registration behavior
  - Mandate singleton pattern for all tool commands
  - Automatic command registration with CommandDispatcher
  - Zero boilerplate in derived classes

#### Initial Implementation Attempt
- **Approach**: Register commands in base constructor using virtual function calls
- **Problem**: C++ construction order - vtable not ready during base constructor
- **Error**: Attempting to call virtual functions (`getCommandPhrase()`, `handleCommand()`) during base construction would call base versions (pure virtual = crash)
- **Root cause**: Virtual dispatch mechanism isn't set up until after all constructors complete

#### Solution: Two-Phase Initialization
- **Pattern**: Lazy registration on first `getInstance()` call
- **Implementation**:
  ```cpp
  static Derived& getInstance() {
      static Derived instance;
      static bool registered = false;
      if (!registered) {
          // Now safe to call virtual functions
          CommandDispatcher::registerCommand(...);
          registered = true;
      }
      return instance;
  }
  ```
- **Benefits**:
  - Object fully constructed before virtual calls
  - Registration happens exactly once
  - Clean, predictable behavior

#### Lambda Capture Issue
- **Problem**: Compiler warning about capturing static local variable by reference
- **Warning**: "capture of variable 'instance' with non-automatic storage duration"
- **Root cause**: Lambda was using `[&instance]` to capture a static variable
- **Solution**: No capture needed - lambda calls `Derived::getInstance()` directly
- **Trade-off**: Slightly more verbose but eliminates confusing capture semantics

#### Design Trade-offs
1. **CRTP (Curiously Recurring Template Pattern)**:
   - **Pro**: Enables compile-time polymorphism for getInstance()
   - **Pro**: Each derived class gets its own static instance
   - **Con**: Template syntax can be confusing
   - **Alternative considered**: Non-template base with virtual getInstance() - rejected due to inability to return derived type

2. **Two-Phase Initialization**:
   - **Pro**: Safe, reliable, works with virtual functions
   - **Pro**: Clear separation between construction and registration
   - **Con**: Slightly more complex than constructor registration
   - **Alternative considered**: Static registration blocks - rejected due to static initialization order fiasco risks

3. **Automatic Registration**:
   - **Pro**: Zero boilerplate in derived classes
   - **Pro**: Impossible to forget registration
   - **Con**: Registration happens implicitly on first use
   - **Alternative considered**: Explicit register() call - rejected to minimize developer error

#### Migration Status
- **Completed**: SecureShell converted to vToolCommand pattern
- **Pending**: SubnetScanner still uses manual registration
- **Usage in main.cpp**: 
  - SecureShell: `SecureShell::getInstance()` triggers auto-registration
  - SubnetScanner: Still uses `std::bind` with manual registration

#### Lessons Learned
- C++ virtual function mechanics are fundamental - can't be bypassed
- Template metaprogramming (CRTP) solves some inheritance limitations
- Lazy initialization is often the cleanest solution for complex initialization
- Compiler warnings about unusual captures should be heeded - usually indicate design smell

### 2025-09-08: vToolCommand Refactor - Compile-time Constants
**Time Spent:** ~20 minutes  
**Issue:** Virtual functions used for compile-time constant strings (command phrase and tip)  
**Solution:** Refactored to use static constexpr members with CRTP pattern  

#### Problem Analysis
- `getCommandPhrase()` and `getCommandTip()` were virtual functions returning static strings
- Virtual function overhead for values that never change at runtime
- Unnecessary dynamic dispatch for compile-time constants

#### Implementation
**Changes:**
- Removed virtual `getCommandPhrase()` and `getCommandTip()` from base class
- Added `static constexpr` members to derived classes:
  ```cpp
  static constexpr const char* COMMAND_PHRASE = "ssh";
  static constexpr const char* COMMAND_TIP = "Connect to SSH host...";
  ```
- Base class accesses via `Derived::COMMAND_PHRASE` in template instantiation
- Keeps `handleCommand()` virtual for runtime polymorphism

#### Design Benefits
- **Performance**: Eliminates virtual function overhead for constants
- **Compile-time enforcement**: Missing constants cause compile error at template instantiation
- **True constexpr**: Values are actual compile-time constants
- **Clean separation**: Runtime polymorphism only where needed

#### Enforcement Mechanism
- Compiler fails at template instantiation if constants not defined
- Error occurs when `getInstance()` is referenced in code
- Not at class definition (like pure virtual) but at point of use
- Ensures design contract while allowing compile-time optimization

**Rationale:** User correctly identified that command metadata is always known at compile time, making virtual functions unnecessary overhead. CRTP pattern maintains design contract enforcement while enabling true compile-time constants.

### 2025-09-08: Winsock Initialization Centralization
**Time Spent:** ~10 minutes  
**Issue:** Multiple components (SecureShell, SubnetScanner) calling WSAStartup/WSACleanup independently  
**Solution:** Moved Winsock initialization to main.cpp  

#### Changes
- Added WSAStartup(MAKEWORD(2, 2)) at beginning of main()
- Added WSACleanup() before program exit
- Removed WSAStartup/WSACleanup from SecureShell constructor/destructor
- Removed WSAStartup/WSACleanup from SubnetScanner::find_hosts()

#### Benefits
- **Single initialization point**: Follows Windows best practice
- **No resource conflicts**: Prevents one component from cleaning up while another needs Winsock
- **Cleaner architecture**: Infrastructure setup at program level, not component level
- **Version consistency**: All components use same Winsock version (2.2)

**Rationale:** User correctly noted that Windows documentation recommends single WSAStartup per process. While Windows reference counts these calls, centralizing initialization is cleaner and prevents potential issues.

### 2025-09-08: vToolCommand Enhancement
**Addition:** Protected save_results() method for file output
- Simple file writing functionality
- Takes filename and data as const string references
- Returns void, outputs status to console
- Available to all derived tool commands

### 2025-09-08: SubnetScanner vToolCommand Migration
**Change:** SubnetScanner now inherits from vToolCommand<SubnetScanner>
- Added static constexpr COMMAND_PHRASE and COMMAND_TIP
- Converted to singleton pattern via getInstance()
- Auto-registers with CommandDispatcher on first use
- Consistent with SecureShell implementation

### 2025-09-11: Per-Command Logging System Design
**Time Spent:** ~30 minutes design session  
**Problem:** Need to capture all cout output to individual log files for each command execution  

#### Requirements Analysis
- Each command execution (scan, ssh, etc.) should generate its own timestamped log file
- Must capture ALL cout output without modifying existing command implementations  
- Should be transparent to command code (no changes needed)

#### Design Decision: Contained within CommandDispatcher
**Rationale:**  
- Minimal code footprint (~50 lines total)
- Single point of control for logging lifecycle
- No new files or dependencies needed
- Follows KISS principle (user preference)

#### Architecture
**TeeStreambuf nested class:**
- Custom streambuf that inherits from `std::streambuf`
- Overrides `overflow()` and `sync()` methods
- Writes to both original cout and log file simultaneously

**Logging lifecycle:**
- `startLogging(commandName)`: Creates log file, redirects cout
- `stopLogging()`: Restores original cout, closes log file
- Called automatically by `processCommand()` wrapper

**Log file format:**
- Directory: `logs/`
- Naming: `<command>_YYYYMMDD_HHMMSS.log`
- Example: `logs/scan_20250911_143022.log`

#### Implementation Status
- **Completed:** Header file updated with complete infrastructure
- **Pending:** Implementation of TeeStreambuf methods
- **Pending:** Implementation of startLogging()/stopLogging()
- **User responsibility:** Will complete implementation

#### Benefits
- No changes required to existing command code
- All cout output automatically captured
- Clean separation of concerns
- Easy to enable/disable logging
- Thread-safe design for multi-threaded commands

### 2025-09-12: Multi-Session SSH Architecture Design
**Time Spent:** ~45 minutes planning and design
**Decision:** Implement multi-session SSH with concurrent operations support
**User Requirements:**
- Multiple SSH sessions maintained simultaneously
- Ability to switch between sessions
- Network scanning while SSH sessions remain connected
- Precise logging per session

#### Architecture Analysis
**Option A: Separate CommandDispatcher Thread**
- CommandDispatcher runs on dedicated thread
- Always available for routing
- Complex synchronization required

**Option B: Main Thread Blocking**
- Simple implementation
- Limited to single session
- Poor scalability

**Final Decision: Modified Option A with SessionManager**
- Each SSH session gets dedicated thread
- SessionManager coordinates all sessions
- Main thread handles routing and output
- Non-blocking operations throughout

#### Design Components

**SessionManager Class:**
```cpp
struct Session {
    uint32_t id;
    string hostname;
    thread sessionThread;
    shared_ptr<SecureShell> shell;
    queue<string> inputQueue;
    queue<string> outputQueue;
    mutex queueMutex;
    bool active;
};
```

**New Commands:**
- `sessions` - List all active SSH sessions
- `switch <id>` - Switch to specific session
- `kill <id>` - Terminate specific session
- `Ctrl+Z` - Detach from current session

**Threading Model:**
- Main thread: Routes input, flushes output
- InputHandler thread: Collects keyboard input
- Session threads: One per SSH connection
- Scanner thread: Temporary for subnet scans

**Benefits:**
1. True concurrency - scan while connected
2. Session persistence - no reconnection needed
3. Clean separation - each session isolated
4. Precise logging - per-session log files
5. Future-proof - supports Telnet, Serial, SNMP

#### Implementation Plan
**Phase 1: Foundation**
- Create SessionManager class
- Refactor SecureShell for persistent connections
- Implement single session with thread

**Phase 2: Multi-Session**
- Session creation and management
- Input/output queue system
- Session switching logic

**Phase 3: Concurrent Operations**
- Background scanning support
- Output multiplexing
- Notification system

**Phase 4: Polish**
- Session status display
- Error recovery
- Resource cleanup

**Learning Goals:**
- User wants to learn C++ patterns through implementation
- Focus on threading, smart pointers, RAII
- Build understanding of message passing between threads
- Practice with STL containers (queues, maps)

**Next Steps:**
1. Create SessionManager.h/cpp files
2. Refactor SecureShell for persistent mode
3. Update CommandDispatcher for session routing
4. Implement Phase 1 together for learning

### 2025-09-15: Logging System Refactor - From Nested to Standalone Class
**Time Spent:** ~2 hours design and implementation
**Problem:** Need flexible logging system for command output capture
**Evolution:** From nested class to standalone component to future vToolCommand integration

#### Initial Implementation
- **TeeStreambuf as nested class** in CommandDispatcher
- Captured ALL command output including built-ins (unintended)
- Single global logger instance

#### Refactoring Journey

**Phase 1: Understanding streambuf vs ostream**
- `streambuf`: Low-level buffer for actual I/O operations
- `ostream`: High-level interface using streambuf internally
- Decision: Use ofstream with `rdbuf()` access for file operations

**Phase 2: File Lifetime Management**
- **Option 1**: Keep file open indefinitely
  - Issues: Buffering risks, resource consumption
- **Option 2**: Open/close per command (selected)
  - Benefits: Data safety, clean resource management
  - Implementation: Open on command start, flush immediately, close on completion

**Phase 3: Pointer Safety Improvements**
- **Problem**: Raw pointer passed to TeeStreambuf could become dangling
- **Initial design**: CommandDispatcher owned unique_ptr, passed raw pointer
- **Risk**: `reset()` would invalidate pointer in TeeStreambuf
- **Solution**: Move ownership into TeeStreambuf itself
  - TeeStreambuf owns the ofstream via unique_ptr
  - Eliminates possibility of dangling pointers
  - Single responsibility: TeeStreambuf manages entire file lifecycle

**Phase 4: Extract to Standalone Class**
- **Created LoggingStreambuf.h/cpp** as independent component
- **Benefits**:
  - Better modularity and reusability
  - Clean separation of concerns
  - Can be used by any component, not just CommandDispatcher

#### Current Architecture

**LoggingStreambuf Features:**
- Inherits from `std::streambuf` for cout interception
- Owns file management via `unique_ptr<ofstream>`
- Creates timestamped directory structure: `logs/YYYYMMDD/`
- Real-time flushing for immediate output
- Thread-safe design for future multi-command scenarios

**Integration:**
- CommandDispatcher creates single LoggingStreambuf instance
- Redirects cout on initialization
- Calls `startLogging(commandName)` before each command
- Calls `stopLogging()` after command completion

#### TODO: Move Logging to vToolCommand

**Problem:** Built-in commands (help, quit) are being logged unintentionally

**Proposed Solution:**
- Each vToolCommand instance creates its own LoggingStreambuf
- Lifecycle matches command execution perfectly
- Built-in commands naturally excluded (don't inherit from vToolCommand)

**Implementation Plan:**
```cpp
class vToolCommand {
    std::unique_ptr<LoggingStreambuf> m_logger;
    std::streambuf* m_original_cout;
    
    vToolCommand(const std::string& name) {
        m_logger = std::make_unique<LoggingStreambuf>();
        m_logger->startLogging(name);
        m_original_cout = std::cout.rdbuf(m_logger.get());
    }
    
    ~vToolCommand() {
        std::cout.rdbuf(m_original_cout);
        m_logger->stopLogging();
    }
};
```

**Benefits:**
- Automatic logging for all tool commands
- No logging for built-in commands
- Support for concurrent commands (each has own logger)
- Clean RAII pattern for resource management

#### Lessons Learned
1. **Ownership clarity is critical** - avoid split ownership scenarios
2. **RAII principles** prevent resource leaks and dangling pointers
3. **Separation of concerns** - logging is independent of command dispatch
4. **Evolution is natural** - design improved through iterative refinement

### 2025-09-15: Logging System Complete Implementation
**Time Spent:** ~30 minutes final implementation
**Status:** ✅ COMPLETE - Logging system fully integrated into vToolCommand

#### Final Implementation Details

**LogStreambuf Class:**
- Renamed from LoggingStreambuf to LogStreambuf (cleaner naming)
- Takes title parameter in constructor for command identification
- Creates timestamped directories: `logs/YYYYMMDD/`
- File naming: `title_details_HHMMSS.txt`
- Added `sanitize_for_windows_path()` to handle special characters

**Static Original Buffer Management:**
```cpp
static std::streambuf* s_cout_original_buf;  // Shared across all instances

// In constructor:
if(!s_cout_original_buf){
    s_cout_original_buf = std::cout.rdbuf();
}

// In stopLogging():
if (std::cout.rdbuf() == this){  // Safety check
    std::cout.rdbuf(s_cout_original_buf);
}
```

**vToolCommand Integration:**
```cpp
// In getInstance():
if(!instance.m_log){
    instance.m_log = std::make_unique<LogStreambuf>(Derived::COMMAND_PHRASE);
}

// Lambda wrapper for command execution:
[](const std::vector<std::string>& args) {
    Derived &instance = Derived::getInstance();
    instance.m_log->startLogging(args[0]);  // Use first arg as details
    instance.handleCommand(args);
    instance.m_log->stopLogging();
}
```

**CommandDispatcher Changes:**
- Removed ALL logging code - complete separation
- Changed command handlers from `bool` to `void` return
- Added public static `s_running` flag for program control
- Built-in commands (help, quit, exit) set `s_running` flag

#### Key Design Achievements

1. **Selective Logging**: Only tool commands are logged, built-ins are not
2. **Zero Boilerplate**: Derived classes need no logging code
3. **Safety Features**: 
   - Static buffer prevents multiple saves
   - Safety check prevents incorrect restoration
   - Supports overlapping logger lifetimes
4. **Clean Architecture**:
   - CommandDispatcher has no logging responsibility
   - LogStreambuf is fully standalone
   - vToolCommand handles its own logging lifecycle

#### Benefits Realized
- Automatic logging for all tool commands
- No logging overhead for built-in commands
- Support for future concurrent commands
- Robust handling of edge cases
- Clean separation of concerns achieved

*Last Updated: 2025-09-15 - Document maintained for human-readable project history and decisions*
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

*Last Updated: 2025-09-06 - Work-stealing thread pool implementation completed*
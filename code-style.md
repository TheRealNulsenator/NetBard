# C++ Code Style Guide

## General Principles
- Clear, readable code over clever tricks
- Self-documenting code through descriptive naming
- Code should "flow naturally" when read
- Consistency throughout the codebase
- Standard, widely-accepted C++ conventions

## Naming Conventions
- **Constants**: UPPER_SNAKE_CASE (e.g., `QUIT_KEY`, `MAX_BUFFER_SIZE`)
- **Functions**: camelCase (e.g., `processInput`, `connectToServer`)
  - Note: Some functions use snake_case (e.g., `find_hosts`, `address_to_bits`)
- **Classes**: PascalCase (e.g., `NetworkMapper`, `ConnectionHandler`)
- **Variables**: Descriptive camelCase or snake_case - prefer clarity over brevity
  - Good: `commandHandlerIterator`, `shouldContinueRunning`, `subnet_hosts`
  - Avoid: `it`, `tmp`, `val`
- **Private members**: Either m_ prefix or snake_case (e.g., `m_data`, `m_isConnected`, `subnet_cider`, `subnet_hosts`)

## Formatting
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Opening brace on same line
- **Line length**: 100 characters max
- **Spaces**: Around operators, after commas

## Best Practices
- No magic numbers or characters - use named constants/arrays
- One declaration per line
- Initialize variables at declaration when possible
- Use const where appropriate (including function return types where values shouldn't change)
- Prefer nullptr over NULL
- **Guard clauses**: Use early returns/continue to flatten nested code
- **Extract complex logic**: Move implementation details to separate functions
- **Function focus**: Keep functions focused on their primary purpose
- **Buffer reuse**: Pass buffers as parameters to avoid repeated allocations
- **Validation**: Validate input parameters early and return meaningful error indicators
- **Bit manipulation**: Use explicit types (uint32_t) and bitwise operations for network address handling
- **Resource optimization**: Initialize expensive resources (WSAStartup, handles) once and reuse
- **Resource management**: Always clean up handles/resources in reverse order of acquisition
- **Performance**: Pass handles/resources as parameters when reusable across iterations

## Parameter Passing Guidelines
- **Pass by const reference**: For objects (std::string, std::vector, custom classes)
  - Example: `void process(const std::string& name)` not `void process(const std::string name)`
- **Pass by value**: For primitives (int, bool, char, pointers, uint32_t)
  - Example: `void setFlag(bool enabled)` is fine
- **Why it matters**: Copying strings/vectors is expensive; copying 4-byte ints is cheap
- **Rule of thumb**: If sizeof(type) > sizeof(pointer), use const reference

## Comments
- Use // for single-line comments
- Place comments above the code they describe
- Keep comments concise and meaningful
- **Inline comments**: Use sparingly, only for non-obvious logic
- Comments on same line as code for brief clarifications

## Headers
- **Include guards**: Use traditional guards, not #pragma once
  - Format: `HEADER_NAME_H` matching the filename
  - Example: `SUBNET_SCANNER_H` for SubnetScanner.h
- **Include optimization**:
  - Headers: Only include what's directly needed
  - Implementation: Don't re-include what's in the header
  - Remove unused includes to reduce compile time
  - Add explicit includes for types (e.g., `<cstdint>` for uint32_t)
- **Include order**:
  - Own header first (in .cpp files)
  - System headers
  - Project headers
  - Alphabetical within each group
- **Windows-specific**: 
  - MUST include `<winsock2.h>` before `<windows.h>`
  - windows.h includes old winsock.h which conflicts with winsock2.h

## Refactoring Patterns
- **Reduce nesting**: Use guard clauses to handle edge cases first
- **Extract helper functions**: Move repetitive or complex logic to dedicated functions
- **Clear separation**: Keep high-level flow separate from implementation details

## Threading Patterns
- **Thread creation**: Use `emplace_back` to avoid copy construction
- **Lambda captures**: 
  - `[&]` for reference capture when threads need shared state
  - Be careful: Don't capture loop variables by reference in thread creation loops
  - Create resources inside lambda to avoid reference capture issues
- **Mutex usage**: Always use `lock_guard` for RAII-style locking
- **Critical sections**: Minimize scope of mutex locks
- **Resource per thread**: Create thread-local resources (e.g., ICMP handles) inside thread lambda
- **Thread joining**: Always join threads before function exit
- **Shared data protection**: Use mutex for any shared data structure modifications

## Work-Stealing Thread Pool Pattern
- **Fixed thread pool**: Create all threads upfront, not one per task
- **Atomic work distribution**: Use `std::atomic<int>` with `fetch_add` for thread-safe indexing
- **Work loop**: Each thread loops until no work remains
  ```cpp
  while (true) {
      int index = work_index.fetch_add(1);
      if (index >= work_items.size()) break;
      // process work_items[index]
  }
  ```
- **Benefits**: No thread creation overhead, automatic load balancing, controlled concurrency
- **Gotchas**: 
  - Don't read-then-increment atomics (race condition)
  - Create handles/resources inside thread, not outside

## Example
```cpp
const int MAX_CONNECTIONS = 100;
const int CHECK_INTERVAL_MS = 100;  // Descriptive constant names

class NetworkManager {
public:
    NetworkManager();
    bool connect(const std::string& address);
    
private:
    int m_connectionCount;
    bool m_isActive;
    
    // Helper functions for complex operations
    std::string waitForResponse(Channel* channel, char* buffer);
};

// Guard clause example
void processData(int value) {
    if (value < 0) return;  // Guard clause
    if (!isValid(value)) return;  // Another guard
    
    // Main logic here, not nested
    performOperation(value);
}
```
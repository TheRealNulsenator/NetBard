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

## Refactoring Patterns
- **Reduce nesting**: Use guard clauses to handle edge cases first
- **Extract helper functions**: Move repetitive or complex logic to dedicated functions
- **Clear separation**: Keep high-level flow separate from implementation details

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
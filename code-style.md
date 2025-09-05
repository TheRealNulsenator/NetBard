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
- **Classes**: PascalCase (e.g., `NetworkMapper`, `ConnectionHandler`)
- **Variables**: Descriptive camelCase - prefer clarity over brevity
  - Good: `commandHandlerIterator`, `shouldContinueRunning`
  - Avoid: `it`, `tmp`, `val`
- **Private members**: m_ prefix (e.g., `m_data`, `m_isConnected`)

## Formatting
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Opening brace on same line
- **Line length**: 100 characters max
- **Spaces**: Around operators, after commas

## Best Practices
- No magic numbers or characters - use named constants/arrays
- One declaration per line
- Initialize variables at declaration when possible
- Use const where appropriate
- Prefer nullptr over NULL
- **Guard clauses**: Use early returns/continue to flatten nested code
- **Extract complex logic**: Move implementation details to separate functions
- **Function focus**: Keep functions focused on their primary purpose
- **Buffer reuse**: Pass buffers as parameters to avoid repeated allocations

## Comments
- Use // for single-line comments
- Place comments above the code they describe
- Keep comments concise and meaningful
- **Inline comments**: Use sparingly, only for non-obvious logic
- Comments on same line as code for brief clarifications

## Headers
- Include guards or #pragma once
- System headers before project headers
- Alphabetical order within each group

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
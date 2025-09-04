# C++ Code Style Guide

## General Principles
- Clear, readable code over clever tricks
- Consistency throughout the codebase
- Standard, widely-accepted C++ conventions

## Naming Conventions
- **Constants**: UPPER_SNAKE_CASE (e.g., `QUIT_KEY`, `MAX_BUFFER_SIZE`)
- **Functions**: camelCase (e.g., `processInput`, `connectToServer`)
- **Classes**: PascalCase (e.g., `NetworkMapper`, `ConnectionHandler`)
- **Variables**: camelCase (e.g., `userInput`, `connectionCount`)
- **Private members**: m_ prefix (e.g., `m_data`, `m_isConnected`)

## Formatting
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Opening brace on same line
- **Line length**: 100 characters max
- **Spaces**: Around operators, after commas

## Best Practices
- No magic numbers - use named constants
- One declaration per line
- Initialize variables at declaration when possible
- Use const where appropriate
- Prefer nullptr over NULL

## Comments
- Use // for single-line comments
- Place comments above the code they describe
- Keep comments concise and meaningful

## Headers
- Include guards or #pragma once
- System headers before project headers
- Alphabetical order within each group

## Example
```cpp
const int MAX_CONNECTIONS = 100;

class NetworkManager {
public:
    NetworkManager();
    bool connect(const std::string& address);
    
private:
    int m_connectionCount;
    bool m_isActive;
};
```
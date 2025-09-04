# Network Cartographer - Project Log

## Project Overview
**Purpose**: Network mapping and visualization tool  
**Author**: Brad Nulsen, Grantek  
**Started**: January 2025  

---

## Design Decisions

### 2025-01-04: Initial Architecture
- **Decision**: Console-based application with keyboard input handling
- **Rationale**: Start with simple CLI interface, can add GUI later if needed
- **Implementation**: Using Windows console I/O (conio.h) for non-blocking keyboard input

### 2025-01-04: Exit Mechanism
- **Decision**: Use ESC key as program exit trigger
- **Rationale**: Standard convention, doesn't interfere with typical input
- **Alternative considered**: Ctrl+C (rejected - too abrupt, no cleanup opportunity)

---

## Technical Adjustments

### 2025-01-04: Code Quality Standards
- Established code style guide (code-style.md)
- Eliminated magic numbers - using named constants
- Adopted standard C++ naming conventions

---

## Complexity Points

### Console Input Handling
- **Challenge**: Need non-blocking keyboard input on Windows
- **Solution**: Using `_kbhit()` and `_getch()` from conio.h
- **Trade-off**: Platform-specific (Windows only), but simple and effective
- **Future consideration**: May need abstraction layer for cross-platform support

---

## Architecture Evolution

### Phase 1: Foundation (Current)
- Basic console application structure
- Event loop with keyboard handling
- Clean exit mechanism

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

1. **Platform dependency**: Currently Windows-only due to conio.h
2. **Console limitations**: Text-only interface for now

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
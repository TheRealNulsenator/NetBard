#include <iostream>
#include <windows.h>  // Windows API for console input

const char QUIT_KEY = 27;  // ESC key (VK_ESCAPE)

int main() {
    std::cout << "Network Cartographer\nGrantek - Brad Nulsen (2025)" << std::endl;
    std::cout << "\nPress ESC to exit..." << std::endl;
    
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT);
    
    // Keep running until ESC is pressed
    while (true) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
        Sleep(10);  // Small delay to prevent CPU spinning
    }
    
    // Restore console mode
    SetConsoleMode(hStdin, mode);
    
    std::cout << "Exiting..." << std::endl;
    return 0;
}
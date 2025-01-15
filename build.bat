chcp 65001 >nul
g++ -g -Wall -Wextra -Wno-unused-label -std=c++17 -DTARGET_OS_WINDOWS demo.cpp base.cpp -o demo && demo.exe

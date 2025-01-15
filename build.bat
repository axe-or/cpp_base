@echo off
chcp 65001
g++ -g -Wall -std=c++17 -DTARGET_OS_WINDOWS demo.cpp base.cpp -o demo && demo.exe


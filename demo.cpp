#include "base.hpp"
#include <iostream>

template<typename T>
void print(T x){
    std::cout << x << '\n';
}

template<typename T, typename ...Args>
void print(T x, Args... rest){
    std::cout << std::boolalpha;
    std::cout << x << ' ';
    print(rest...);
}

int main(){
    print(">", 293, true);
    
    return 0;
}
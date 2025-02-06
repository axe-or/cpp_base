#include "base.hpp"
#include <iostream>

using mem::Arena;
namespace str = strings;

template<typename T>
void print(T a){
	std::boolalpha(std::cout);
	std::cout << a << '\n';
}

template<typename T, typename ... Args>
void print(T a, Args&& ... args){
	std::boolalpha(std::cout);
	std::cout << a << ' ';
	print(args...);
}

std::ostream& operator<<(std::ostream& os, String s){
	os.write((char const*)s.raw_data(), s.len());
	return os;
}

int main(){
	String s = "Chup Chups";

	print(str::starts_with(s, "Chup"));
	print(str::ends_with(s, "Chups"));
	print(str::starts_with(s, ""));
	print(str::ends_with(s, ""));
	print(str::starts_with(s, s));
	print(str::ends_with(s, s));

	print(str::starts_with(s, "Chu"));
	print(str::ends_with(s, "ps"));

	return 0;
}

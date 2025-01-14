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

std::ostream& operator<<(std::ostream& os, String s){
	for(isize i = 0; i < s.size(); i ++){
		os << s[i];
	}
	return os;
}

struct Codepoint {
	rune value;
	explicit Codepoint(rune r) : value(r){}
};

std::ostream& operator<<(std::ostream& os, Codepoint r){
	auto res = utf8::encode(r.value);
	char buf[5] = {0};
	mem::copy(buf, res.bytes, res.len);
	os << buf;
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, Slice<T> s){
	if(s.size() == 0){
		os << "[]";
		return os;
	}

	os << '[';
	for(isize i = 0; i < (s.size() - 1); i ++){
		os << s[i] << ' ';
	}
	os << s[s.size() - 1] << ']';
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, Dynamic_Array<T> s){
	if(s.size() == 0){
		os << "(size:0 cap:" << s.cap() << ")[]";
		return os;
	}

	os << "(size:" << s.size() << " cap:" << s.cap() << ")[";
	for(isize i = 0; i < (s.size() - 1); i ++){
		os << s[i] << ' ';
	}
	os << s[s.size() - 1] << ']';
	return os;
}

#include "tests.cpp"

int main(){
	test_slice();
	test_string();
	test_dynamic_array();
    return 0;
}


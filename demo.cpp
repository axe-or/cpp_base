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

struct rune_wrapper {
	rune r;
	explicit rune_wrapper(rune r) : r(r){}
};

std::ostream& operator<<(std::ostream& os, rune_wrapper r){
	auto res = utf8::encode(r.r);
	char buf[5] = {0};
	mem::copy(buf, res.bytes, res.len);
	os << buf;
	return os;
}

int main(){
	String s = "Hello, 世界";
    print(s);

	auto it = s.iterator();
	for(rune r = it.next(); r != 0; r = it.next()){
		print(rune_wrapper(r));
	}
    return 0;
}

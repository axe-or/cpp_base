#ifndef _debug_print_cpp_include_
#define _debug_print_cpp_include_

#include <iostream>

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

static inline
std::ostream& operator<<(std::ostream& os, String s){
	os.write((char const*)s.raw_data(), s.len());
	return os;
}


template<typename T>
std::ostream& operator<<(std::ostream& os, Slice<T> s){
	os << '[';
	for(Size i = 0; i < len(s); i++){
		if(i > 0) os << ' ';
		os << s[i];
	}
	os << ']';
	return os;
}

#endif /* Include guard */

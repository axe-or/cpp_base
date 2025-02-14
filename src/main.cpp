#include "base.hpp"
#include "debug_print.cpp"

int main(){
	auto allocator = mem_mi_allocator();

	auto map = map_create<String, I32>(allocator, 32);
	set(&map, "Cu", 69);

	return 0;
}


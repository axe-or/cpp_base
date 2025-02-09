#include "base.hpp"
#include "debug_print.cpp"

int main(){
	auto arena = arena_create_virtual(100'000);
	auto allocator = arena_allocator(&arena);

	auto arr = dynamic_array_create<I32>(allocator);
	append(&arr, 1);
	append(&arr, 2);
	append(&arr, 3);
	append(&arr, 69);
	print(slice(arr));
	return 0;
}

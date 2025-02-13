#include "base.hpp"
#include "debug_print.cpp"

int main(){
	auto arena = arena_create_virtual(100'000);
	auto allocator = arena_allocator(&arena);

	auto arr = dynamic_array_create<I32>(allocator, 2);
	insert(&arr, 0, 9);
	insert(&arr, 0, 6);
	insert(&arr, 0, 0);
	insert(&arr, len(arr), 0);
	insert(&arr, 2, 0);

	print(slice(arr));
	return 0;
}


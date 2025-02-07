#include "base.hpp"

int main(){
	print("Init");
	auto main_arena = arena_create_virtual(64 * mem_MiB);
	auto arr = dynamic_array_create<F32>(&main_arena);
	append(&arr, 6);
	append(&arr, 9);
	append(&arr, 4);
	append(&arr, 2);
	append(&arr, 0);
	auto s = slice(arr)[{2, 4}];
	print(slice(arr));
	print(s);

	return 0;
}

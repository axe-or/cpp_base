#include "base.hpp"

using mem::Arena;
namespace str = strings;


int main(){
	print("Init");
	auto main_arena = Arena::create_virtual(5 * mem::MiB);
	auto arr = DynamicArray<F32>::create(&main_arena);
	arr.push(6);
	arr.push(9);
	arr.push(4);
	arr.push(2);
	arr.push(0);
	print(arr.slice());

	return 0;
}

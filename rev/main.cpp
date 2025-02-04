#include "base.hpp"

int main(){
	auto arena = mem::Arena::from_virtual(20'000);
	defer(arena.destroy());

	auto nums = arena.make<F32>(40);
	for(int i = 0; i < nums.len(); i++){
		nums[i] = 1.0 / (i + 1);
		printf("%.8f ", nums[i]);
	}

	return 0;
}

#include "base.hpp"

Slice<Byte> allocate_something(Allocator auto* a, Size nbytes){
	void* buf = mem_alloc(a, nbytes, 1);
	return slice<Byte>((Byte*)buf, nbytes);
}

int main(){

	return 0;
}

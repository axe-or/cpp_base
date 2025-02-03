#include "base.hpp"

int main(){
	void* p = mem::virt::reserve(3000);
	defer(p = 0);
	return 0;
}

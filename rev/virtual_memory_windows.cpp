#include "base.hpp"
extern "C" {
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
}

namespace mem::virt {
static inline
bool valid_ptr_and_size(void* ptr, Size size){
	return ((Uintptr(ptr) & (page_size - 1)) == 0) && ((size & (page_size - 1)) == 0);
}

void* reserve(Size nbytes){
	nbytes = mem::align_forward_size(nbytes, page_size);
	return VirtualAlloc(nullptr, nbytes, MEM_RESERVE, PAGE_NOACCESS);
}

void release(void* pointer, Size nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	VirtualFree(pointer, nbytes, MEM_RELEASE);
}

void* commit(void* pointer, Size nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	void* p = VirtualAlloc(pointer, nbytes, MEM_COMMIT, PAGE_READWRITE);
	return p;
}

void decommit(void* pointer, Size nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	VirtualFree(pointer, nbytes, MEM_DECOMMIT);
}

static inline
DWORD protect_flags(U8 prot){
	bool write = prot & protection_write;
	bool read  = prot & protection_read;
	bool exec  = prot & protection_execute;

	if(read && write && exec){
		return PAGE_EXECUTE_READWRITE;
	}

	if(read && exec){
		return PAGE_EXECUTE_READ;
	}

	if(read && write){
		return PAGE_READWRITE;
	}

	if(read){
		return PAGE_READONLY;
	}

	debug_assert(false, "Invalid memory protection set");
	return 0;
}

bool protect(void* pointer, Size nbytes, U32 prot){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	[[maybe_unused]] DWORD old;
	return VirtualProtect(pointer, nbytes, protect_flags(prot), &old);
}

}

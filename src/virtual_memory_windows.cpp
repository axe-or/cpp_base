#include "base.hpp"
extern "C" {
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
}

static inline
bool valid_ptr_and_size(void* ptr, isize size){
	return ((uintptr(ptr) & (mem_page_size - 1)) == 0) && ((size & (mem_page_size - 1)) == 0);
}

void* virtual_reserve(isize nbytes){
	nbytes = mem_align_forward_size(nbytes, mem_page_size);
	return VirtualAlloc(nullptr, nbytes, MEM_RESERVE, PAGE_NOACCESS);
}

void virtual_release(void* pointer, isize nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	VirtualFree(pointer, nbytes, MEM_RELEASE);
}

void* virtual_commit(void* pointer, isize nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	void* p = VirtualAlloc(pointer, nbytes, MEM_COMMIT, PAGE_READWRITE);
	return p;
}

void virtual_decommit(void* pointer, isize nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	VirtualFree(pointer, nbytes, MEM_DECOMMIT);
}

static inline
DWORD protect_flags(u32 prot){
	bool write = prot & mem_protection_write;
	bool read  = prot & mem_protection_read;
	bool exec  = prot & mem_protection_execute;

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

bool protect(void* pointer, isize nbytes, u32 prot){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	[[maybe_unused]] DWORD old;
	return VirtualProtect(pointer, nbytes, protect_flags(prot), &old);
}


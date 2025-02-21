#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "base.hpp"
#include <sys/mman.h>
#include <unistd.h>

static inline
bool valid_ptr_and_size(void* ptr, isize size){
	return ((uintptr(ptr) & (mem_page_size - 1)) == 0) && ((size & (mem_page_size - 1)) == 0);
}

static inline
u32 protect_flags(u8 prot){
	u32 flag = 0;
	if(prot & mem_protection_execute)
		flag |= PROT_EXEC;
	if(prot & mem_protection_read)
		flag |= PROT_READ;
	if(prot & mem_protection_write)
		flag |= PROT_WRITE;
	return flag;
}

void* virtual_reserve(isize nbytes){
	nbytes = mem_align_forward_size(nbytes, mem_page_size);
	void* ptr = mmap(NULL, nbytes, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	return ptr;
}

void virtual_release(void* pointer, isize nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	munmap(pointer, nbytes);
}

void* virtual_commit(void* pointer, isize nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	if(mprotect(pointer, nbytes, PROT_READ | PROT_WRITE) < 0){
		return NULL;
	}
	return pointer;	
}

void virtual_decommit(void* pointer, isize nbytes){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	mprotect(pointer, nbytes, PROT_NONE);
	madvise(pointer, nbytes, MADV_FREE);
}

bool virtual_protect(void* pointer, isize nbytes, u8 prot){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	u32 flags = protect_flags(prot);
	return mprotect(pointer, nbytes, flags) >= 0;
}


#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "base.hpp"
#include <sys/mman.h>
#include <unistd.h>

namespace mem::virt {
static inline
bool valid_ptr_and_size(void* ptr, Size size){
	return ((Uintptr(ptr) & (page_size - 1)) == 0) && ((size & (page_size - 1)) == 0);
}

static inline
U32 protect_flags(U8 prot){
	U32 flag = 0;
	if(prot & protection_execute)
		flag |= PROT_EXEC;
	if(prot & protection_read)
		flag |= PROT_READ;
	if(prot & protection_write)
		flag |= PROT_WRITE;
	return flag;
}

void* reserve(Size nbytes){
	nbytes = align_forward_size(nbytes, page_size);
	void* ptr = mmap(NULL, nbytes, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	return ptr;
}

void release(void* ptr, Size len){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	munmap(ptr, len);
}

void* commit(void* ptr, Size len){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	if(mprotect(ptr, len, PROT_READ | PROT_WRITE) < 0){
		return NULL;
	}
	return ptr;	
}

void decommit(void* ptr, Size len){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	mprotect(ptr, len, PROT_NONE);
	madvise(ptr, len, MADV_FREE);
}

bool protect(void* ptr, Size len, U8 prot){
	debug_assert(valid_ptr_and_size(pointer, nbytes), "Pointer and allocation size must be page aligned");
	U32 flags = protect_flags(prot);
	return mprotect(ptr, len, flags) >= 0;
}
}



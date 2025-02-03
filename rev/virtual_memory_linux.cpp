#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "base.hpp"
#include <sys/mman.h>
#include <unistd.h>

// void virtual_init(){
// 	static bool initialized = false;
// 	if(!initialized){
// 		U32 page_size = getpagesize();
// 		if(page_size != VIRTUAL_PAGE_SIZE){
// 			panic("Virtual memory constraints were not satisfied.");
// 		}
// 		initialized = true;
// 	}
// }

namespace mem::virt {

void* reserve(Size nbytes){
	nbytes = align_forward_size(nbytes, page_size);
	void* ptr = mmap(NULL, nbytes, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	return ptr;
}

void release(void* ptr, Size len){
	ensure(((Uintptr)ptr & (page_size - 1)) == 0, "Pointer is not aligned to page boundary");
	munmap(ptr, len);
}

void* commit(void* ptr, Size len){
	ensure(((Uintptr)ptr & (page_size - 1)) == 0, "Pointer is not aligned to page boundary");
	if(mprotect(ptr, len, PROT_READ | PROT_WRITE) < 0){
		return NULL;
	}
	return ptr;	
}

void decommit(void* ptr, Size len){
	ensure(((Uintptr)ptr & (page_size - 1)) == 0, "Pointer is not aligned to page boundary");
	mprotect(ptr, len, PROT_NONE);
	madvise(ptr, len, MADV_FREE);
}

static inline
U32 _virtual_protect_flags(U8 prot){
	U32 flag = 0;
	if(prot & protection_execute)
		flag |= PROT_EXEC;
	if(prot & protection_read)
		flag |= PROT_READ;
	if(prot & protection_write)
		flag |= PROT_WRITE;
	return flag;
}

bool protect(void* ptr, Size len, U8 prot){
	ensure(((Uintptr)ptr & (page_size - 1)) == 0, "Pointer is not aligned to page boundary");
	U32 flags = _virtual_protect_flags(prot);
	return mprotect(ptr, len, flags) >= 0;
}
}



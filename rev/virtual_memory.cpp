#include "base.hpp"

namespace mem::virt {

PageBlock PageBlock::create(Size nbytes){
	nbytes = align_forward_size(nbytes, page_size);
	void* ptr = reserve(nbytes);
	PageBlock blk = {
		.reserved = (ptr != nullptr) ? nbytes : 0,
		.commited = 0,
		.pointer = ptr,
	};
	return blk;
}

void PageBlock::destroy(){
	release(pointer, reserved);
}

void* PageBlock::push(Size nbytes){
	nbytes = align_forward_size(nbytes, page_size);
	U8* old_ptr = (U8*)pointer + commited;
	void* new_ptr = commit(old_ptr, nbytes);

	if(new_ptr == nullptr){
		return nullptr; /* Memory error */
	}
	commited += nbytes;
	return old_ptr;
}

void PageBlock::pop(Size nbytes){
	nbytes = clamp<Size>(0, nbytes, commited);

	Uintptr base = (Uintptr)pointer;
	// Free pages *after* this location
	Uintptr free_after = base + (commited - nbytes);
	free_after = align_forward_ptr(free_after, page_size);

	Size amount_to_free = (base + commited) - free_after;
	decommit((void*)free_after, amount_to_free);
	commited -= amount_to_free;
}
}

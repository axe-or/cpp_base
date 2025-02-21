#include "base.hpp"

PageBlock PageBlock::make(isize nbytes){
	nbytes = mem_align_forward_size(nbytes, mem_page_size);
	void* ptr = virtual_reserve(nbytes);
	PageBlock blk = {
		.reserved = (ptr != nullptr) ? nbytes : 0,
		.commited = 0,
		.pointer = ptr,
	};
	return blk;
}

void PageBlock::destroy(){
	virtual_release(pointer, reserved);
}

void* PageBlock::push(isize nbytes){
	nbytes = mem_align_forward_size(nbytes, mem_page_size);
	u8* old_ptr = (u8*)pointer + commited;
	void* new_ptr = virtual_commit(old_ptr, nbytes);

	if(new_ptr == nullptr){
		return nullptr; /* Memory error */
	}
	commited += nbytes;
	return old_ptr;
}

void PageBlock::pop(isize nbytes){
	nbytes = clamp<isize>(0, nbytes, commited);

	uintptr base = (uintptr)pointer;
	// Free pages *after* this location
	uintptr free_after = base + (commited - nbytes);
	free_after = mem_align_forward_ptr(free_after, mem_page_size);

	isize amount_to_free = (base + commited) - free_after;
	virtual_decommit((void*)free_after, amount_to_free);
	commited -= amount_to_free;
}

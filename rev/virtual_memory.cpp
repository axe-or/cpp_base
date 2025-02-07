#include "base.hpp"

PageBlock page_block_create(Size nbytes){
	nbytes = mem_align_forward_size(nbytes, mem_page_size);
	void* ptr = virtual_reserve(nbytes);
	PageBlock blk = {
		.reserved = (ptr != nullptr) ? nbytes : 0,
		.commited = 0,
		.pointer = ptr,
	};
	return blk;
}

void page_block_destroy(PageBlock* b){
	virtual_release(b->pointer, b->reserved);
}

void* page_block_push(PageBlock* b, Size nbytes){
	nbytes = mem_align_forward_size(nbytes, mem_page_size);
	U8* old_ptr = (U8*)b->pointer + b->commited;
	void* new_ptr = virtual_commit(old_ptr, nbytes);

	if(new_ptr == nullptr){
		return nullptr; /* Memory error */
	}
	b->commited += nbytes;
	return old_ptr;
}

void page_block_pop(PageBlock* b, Size nbytes){
	nbytes = clamp<Size>(0, nbytes, b->commited);

	Uintptr base = (Uintptr)b->pointer;
	// Free pages *after* this location
	Uintptr free_after = base + (b->commited - nbytes);
	free_after = mem_align_forward_ptr(free_after, mem_page_size);

	Size amount_to_free = (base + b->commited) - free_after;
	virtual_decommit((void*)free_after, amount_to_free);
	b->commited -= amount_to_free;
}

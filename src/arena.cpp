#include "base.hpp"

namespace mem {
static
uintptr arena_required_mem(uintptr cur, isize nbytes, isize align){
	debug_assert(valid_alignment(align), "Alignment must be a power of 2");
	uintptr_t aligned  = align_forward_ptr(cur, align);
	uintptr_t padding  = (uintptr)(aligned - cur);
	uintptr_t required = padding + nbytes;
	return required;
}

void* Arena::alloc(isize size, isize align){
	uintptr base = (uintptr)data;
	uintptr current = (uintptr)base + (uintptr)offset;

	uintptr available = (uintptr)capacity - (current - base);
	uintptr required = arena_required_mem(current, size, align);

	if(required > available){
		return nullptr;
	}

	offset += required;
	void* allocation = &data[offset - size];
	last_allocation = (uintptr)allocation;
	return allocation;
}

void Arena::free_all(){
	offset = 0;
}

void* Arena::resize(void* ptr, isize new_size){
	if((uintptr)ptr == last_allocation){
		uintptr base = (uintptr)data;
		uintptr current = base + (uintptr)offset;
		uintptr limit = base + (uintptr)capacity;
		isize last_allocation_size = current - last_allocation;

		if((current - last_allocation_size + new_size) > limit){
			return nullptr; /* No space left*/
		}

		offset += new_size - last_allocation_size;
		return ptr;
	}

	return nullptr;
}

static
void* arena_allocator_func(
	void* impl,
	Allocator_Op op,
	void* old_ptr,
	isize size,
	isize align,
	u32* capabilities)
{
	auto arena = (Arena*)impl;
	(void)old_ptr;
	using M = Allocator_Op;
	using C = Allocator_Capability;

	switch(op){
		case M::Alloc: {
			return arena->alloc(size, align);
		} break;

		case M::Free_All: {
			arena->free_all();
		} break;

		case M::Resize: {
			return arena->resize(old_ptr, size);
		} break;

		case M::Free: {} break;

		case M::Query: {
			*capabilities = u32(C::Align_Any) | u32(C::Free_All) | u32(C::Alloc_Any) | u32(C::Resize);
		} break;
	}

	return nullptr;
}

Allocator Arena::allocator(){
	Allocator allocator = {
		._impl = this,
		._func = arena_allocator_func,
	};
	return allocator;
}

void arena_init(Arena* a, byte* data, isize len){
	a->capacity = len;
	a->data = data;
	a->offset = 0;
}


} /* Namespace mem */


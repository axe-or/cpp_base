#include "base.hpp"
#include "mimalloc.h"

// void* mi_zalloc_aligned(size_t size, size_t alignment) {}

static
void* mem_mi_allocator_func(
	void*,
	AllocatorOp op,
	void* old_ptr,
	Size old_size,
	Size size,
	Size align,
	U32* capabilities
){
	using M = AllocatorOp;
	using C = AllocatorCapability;

	switch(op){
	case M::Query:{
		*capabilities = U32(C::AlignAny) | U32(C::AllocAny) | U32(C::FreeAny) | U32(C::Resize);
	} break;

	case M::Alloc:
		return mi_zalloc_aligned(size, align);

	case M::Resize:
		return mi_expand(old_ptr, old_size);

	case M::Free: {
		mi_free(old_ptr);
	} break;

	case M::Realloc:
		return mi_realloc_aligned(old_ptr, size, align);

	case M::FreeAll: break;
	}

	return nullptr;
}

Allocator mem_mi_allocator(){
	Allocator a = {
		.data = nullptr,
		.func = mem_mi_allocator_func,
	};
	return a;
}

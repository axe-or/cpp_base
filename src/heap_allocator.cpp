#include "base.hpp"
#include <new>

static
void* mem_heap_allocator_func(
	void*,
	AllocatorMode op,
	void* old_ptr,
	Size old_size,
	Size size,
	Size align,
	U32* capabilities
){
	using M = AllocatorMode;
	using C = AllocatorCapability;
	switch (op) {
		case M::Query: {
			*capabilities = U32(C::AllocAny) | U32(C::FreeAny) | U32(C::AlignAny);
		} break;

		case M::Alloc: {
			Byte* p = new (std::align_val_t(align)) Byte[size];
			[[likely]] if(p != nullptr){
				mem_set(p, 0, size);
			}
			return p;
		}

		case M::Resize: {
			return nullptr;
		}

		case M::Free: {
			Byte* old_p = (Byte*)old_ptr;
			operator delete[](old_p, std::align_val_t(align));
		} break;

		case M::FreeAll: break;

		case M::Realloc: {
			Byte* new_p = new (std::align_val_t(align)) Byte[size];
			Byte* old_p = (Byte*)old_ptr;
			Size nbytes = min(old_size, size);
			[[likely]] if(new_p != nullptr){
				mem_copy_no_overlap(new_p, old_p, nbytes);
				operator delete[](old_p, std::align_val_t(align));
			}
			return new_p;
		}
	}

	return nullptr;
}

Allocator heap_allocator(){
	return Allocator{
		.data = nullptr,
		.func = mem_heap_allocator_func,
	};
}


#include "base.hpp"
#include <new>

static
void* mem_heap_allocator_func(
	void*,
	AllocatorMode op,
	void* old_ptr,
	isize old_size,
	isize size,
	isize align,
	u32* capabilities
){
	using M = AllocatorMode;
	using C = AllocatorCapability;
	switch (op) {
		case M::Query: {
			*capabilities = u32(C::AllocAny) | u32(C::FreeAny) | u32(C::AlignAny);
		} break;

		case M::Alloc: {
			byte* p = new (std::align_val_t(align)) byte[size];
			[[likely]] if(p != nullptr){
				mem_set(p, 0, size);
			}
			return p;
		}

		case M::Resize: {
			return nullptr;
		}

		case M::Free: {
			byte* old_p = (byte*)old_ptr;
			operator delete[](old_p, std::align_val_t(align));
		} break;

		case M::FreeAll: break;

		case M::Realloc: {
			byte* new_p = new (std::align_val_t(align)) byte[size];
			byte* old_p = (byte*)old_ptr;
			isize nbytes = min(old_size, size);
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


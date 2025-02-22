#include "base.hpp"
#include <new>

static
Result<void*, MemoryError> mem_heap_allocator_func(
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

	Result<void*, MemoryError> res = {nullptr, MemoryError::None};

	switch (op) {
		case M::Query: {
			*capabilities = u32(C::AllocAny) | u32(C::FreeAny) | u32(C::AlignAny);
			return res;
		}

		case M::Alloc: {
			res.value = new (std::align_val_t(align)) byte[size];
			[[likely]] if(res.value != nullptr){
				mem_set(res.value, 0, size);
			} else {
				res.error = MemoryError::OutOfMemory;
			}
			return res;
		}

		case M::Resize: {
			res.error = MemoryError::ResizeFailed;
			return res;
		}

		case M::Free: {
			byte* old_p = (byte*)old_ptr;
			operator delete[](old_p, std::align_val_t(align));
			return res;
		};

		case M::FreeAll: {
			return res;
		}

		case M::Realloc: {
			byte* new_p = new (std::align_val_t(align)) byte[size];
			byte* old_p = (byte*)old_ptr;
			isize nbytes = min(old_size, size);
			[[likely]] if(new_p != nullptr){
				mem_copy_no_overlap(new_p, old_p, nbytes);
				operator delete[](old_p, std::align_val_t(align));
			} else {
				res.error = MemoryError::OutOfMemory;
			}
			res.value = new_p;
			return res;
		}
	}

	res.error = MemoryError::UnknownMode;
	return res;
}

Allocator heap_allocator(){
	return Allocator{
		.data = nullptr,
		.func = mem_heap_allocator_func,
	};
}


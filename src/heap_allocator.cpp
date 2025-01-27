#include "base.hpp"

namespace mem {

static void* _heap_allocator_func (
	[[maybe_unused]] void* impl,
	Allocator_Op op,
	void* old_ptr,
	isize size,
	isize align,
	u32* capabilities
){

	using M = Allocator_Op;
	using C = Allocator_Capability;

	switch(op){
		case M::Alloc: {
		    ensure(valid_alignment(align), "Invalid alignment");
			auto ptr = new(std::align_val_t(align)) byte[size];
			return (void*)ptr;
		} break;

		case M::Free_All: break;

		case M::Resize: break;

		case M::Free: {
		    ensure(valid_alignment(align), "Invalid alignment");
			auto p = (byte*)old_ptr;
			::operator delete[](p, std::align_val_t(align));
		} break;

		case M::Query: {
			*capabilities = u32(C::Align_Any) | u32(C::Free_Any) | u32(C::Alloc_Any);
		} break;
	}

	return nullptr;
}

Allocator heap_allocator(){
	Allocator a = {
		._impl = nullptr,
		._func = _heap_allocator_func,
	};
	return a;
}

} /* Namespace mem */

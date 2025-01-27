#include "base.hpp"

namespace mem {
#if !defined(__clang__) && !defined(__GNUC__)
#include <string.h>
#define mem_set_impl             memset
#define mem_copy_impl            memmove
#define mem_copy_no_overlap_impl memcpy
#define mem_compare_impl         memcmp
#else
#define mem_set_impl             __builtin_memset
#define mem_copy_impl            __builtin_memmove
#define mem_copy_no_overlap_impl __builtin_memcpy
#define mem_compare_impl         __builtin_memcmp
#endif

void set(void* p, byte val, isize nbytes){
	mem_set_impl(p, val, nbytes);
}

void copy(void* dest, void const * src, isize nbytes){
	mem_copy_impl(dest, src, nbytes);
}

void copy_no_overlap(void* dest, void const * src, isize nbytes){
	mem_copy_no_overlap_impl(dest, src, nbytes);
}

i32 compare(void const * a, void const * b, isize nbytes){
	return mem_compare_impl(a, b, nbytes);
}

uintptr align_forward_ptr(uintptr p, uintptr a){
	debug_assert(valid_alignment(a), "Invalid memory alignment");
	uintptr mod = p & (a - 1); // Fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

uintptr align_forward_size(isize p, isize a){
	debug_assert(valid_alignment(a), "Invalid size alignment");
	isize mod = p & (a - 1); // Fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

} /* Namespace mem */


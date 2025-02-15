#include "base.hpp"

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

void* mem_alloc(Allocator a, Size nbytes, Size align){
	return a.func(a.data, AllocatorMode::Alloc, nullptr, 0, nbytes, align, nullptr);
}

void* mem_resize(Allocator a, void* ptr, Size new_size){
	return a.func(a.data, AllocatorMode::Resize, ptr, 0, new_size, 0, nullptr);
}

void* mem_realloc(Allocator a, void* ptr, Size old_size, Size new_size, Size align){
	return a.func(a.data, AllocatorMode::Realloc, ptr, old_size, new_size, align, nullptr);
}

void mem_free(Allocator a, void* ptr, Size old_size, Size align){
	a.func(a.data, AllocatorMode::Free, ptr, old_size, 0, align, nullptr);
}

void mem_free_all(Allocator a){
	a.func(a.data, AllocatorMode::FreeAll, 0, 0, 0, 0, nullptr);
}

void mem_set(void* p, Byte val, Size count){
	mem_set_impl(p, val, count);
}

void mem_copy(void* dest, void const * src, Size count){
	mem_copy_impl(dest, src, count);
}

void mem_copy_no_overlap(void* dest, void const * src, Size count){
	mem_copy_no_overlap_impl(dest, src, count);
}

I32 mem_compare(void const * a, void const * b, Size count){
	return mem_compare_impl(a, b, count);
}


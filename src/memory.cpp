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

void* Allocator::alloc(isize nbytes, isize align){
	return this->func(this->data, AllocatorMode::Alloc, nullptr, 0, nbytes, align, nullptr);
}

void* Allocator::resize(void* ptr, isize new_size){
	return this->func(this->data, AllocatorMode::Resize, ptr, 0, new_size, 0, nullptr);
}

void* Allocator::realloc(void* ptr, isize old_size, isize new_size, isize align){
	return this->func(this->data, AllocatorMode::Realloc, ptr, old_size, new_size, align, nullptr);
}

void Allocator::free(void* ptr, isize old_size, isize align){
	this->func(this->data, AllocatorMode::Free, ptr, old_size, 0, align, nullptr);
}

void Allocator::free_all(){
	this->func(this->data, AllocatorMode::FreeAll, 0, 0, 0, 0, nullptr);
}

void mem_set(void* p, byte val, isize count){
	mem_set_impl(p, val, count);
}

void mem_copy(void* dest, void const * src, isize count){
	mem_copy_impl(dest, src, count);
}

void mem_copy_no_overlap(void* dest, void const * src, isize count){
	mem_copy_no_overlap_impl(dest, src, count);
}

i32 mem_compare(void const * a, void const * b, isize count){
	return mem_compare_impl(a, b, count);
}


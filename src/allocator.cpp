#include "base.hpp"

// enum class Allocator_Op : byte {
// 	Query    = 0, // Query allocator's capabilities
// 	Alloc    = 1, // Allocate a chunk of memory
// 	Resize   = 2, // Resize an allocation in-place
// 	Free     = 3, // Mark allocation as free
// 	Free_All = 4, // Mark allocations as free
// 	Realloc  = 5, // Re-allocate pointer
// };
//
// enum class Allocator_Capability : u32 {
// 	Alloc_Any = 1 << 0, // Can alloc any size
// 	Free_Any  = 1 << 1, // Can free in any order
// 	Free_All  = 1 << 2, // Can free all allocations
// 	Resize    = 1 << 3, // Can resize in-place
// 	Align_Any = 1 << 4, // Can alloc aligned to any alignment
// };
//
// // Memory allocator method
// using Allocator_Func = void* (*) (
// 	void* impl,
// 	Allocator_Op op,
// 	void* old_ptr,
// 	isize size,
// 	isize align,
// 	u32* capabilities
// );
//
// // Memory allocator interface
// struct Allocator {
// 	void* _impl{0};
// 	Allocator_Func _func{0};
//
// 	// Get capabilities of allocator as a number, gets capability bit-set
// 	u32 query_capabilites(){
// 		u32 n = 0;
// 		_func(_impl, Allocator_Op::Query, nullptr, 0, 0, &n);
// 		return n;
// 	}
//
// 	// Allocate fresh memory, filled with 0s. Returns NULL on failure.
// 	void* alloc(isize size, isize align){
// 		return _func(_impl, Allocator_Op::Alloc, nullptr, size, align, nullptr);
// 	}
//
// 	// Re-allocate memory in-place without changing the original pointer. Returns
// 	// NULL on failure.
// 	void* resize(void* ptr, isize new_size){
// 		return _func(_impl, Allocator_Op::Resize, ptr, new_size, 0, nullptr);
// 	}
//
// 	// Free pointer to memory, includes alignment information, which is required for
// 	// some allocators, freeing NULL is a no-op
// 	void free(void* ptr, isize size, isize align){
// 		_func(_impl, Allocator_Op::Free, ptr, size, align, nullptr);
// 	}
//
// 	// Free all pointers owned by allocator
// 	void free_all(){
// 		_func(_impl, Allocator_Op::Free_All, nullptr, 0, 0, nullptr);
// 	}
//
// };
// // Helper to create any type
// template<typename T>
// T* make(){
// 	auto p = (T*)this->alloc(sizeof(T), alignof(T));
// 	if(p != nullptr){
// 		new (p) T();
// 	}
// 	return p;
// }
//
// // Helper to create any type
// template<typename T>
// Slice<T> make(isize n){
// 	auto p = (T*)this->alloc(n * sizeof(T), alignof(T));
// 	if(p != nullptr){
// 		for(isize i = 0; i < n; i++){
// 			new (&p[i]) T();
// 		}
// 	}
// 	return Slice<T>::from_pointer(p, n);
// }
//
// // Helper to deallocate any trivial type
// template<typename T>
// void destroy(T* p){
// 	this->free((void*)p, sizeof(T), alignof(T));
// }
//
// // Helper to deallocate any trivial type
// template<typename T>
// void destroy(Slice<T> s){
// 	T* buf = s.raw_data();
// 	isize n = s.size();
// 	this->free((void*)buf, n * sizeof(T), alignof(T));
// }

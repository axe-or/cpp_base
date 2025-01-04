#pragma once

//// Includes //////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdalign.h>
#include <tgmath.h>
#include <limits.h>
#include <float.h>

#include <atomic>
#include <new>

//// Essentials ////////////////////////////////////////////////////////////////
#define null NULL

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using uint = unsigned int;
using byte = uint8_t;
using rune = i32;

using isize = ptrdiff_t;
using usize = size_t;

using uintptr = uintptr_t;

using f32 = float;
using f64 = double; 

using cstring = char const *;

// Swap bytes around, useful for when dealing with endianess
template<typename T>
void swap_bytes(T* data){
	isize len = sizeof(T);
	for(isize i = 0; i < (len / 2); i += 1){
		byte temp = data[i];
		data[i] = data[len - (i + 1)];
		data[len - (i + 1)] = temp;
	}
}

template<typename T>
T abs(T x){
	return (x < static_cast<T>(0)) ? - x : x;
}

template<typename T>
T min(T a, T b){ return a < b ? a : b; }

template<typename T, typename ...Args>
T min(T a, T b, Args... rest){
	if(a < b)
		return min(a, rest...);
	else
		return min(b, rest...);
}

template<typename T>
T max(T a, T b){
	return a > b ? a : b;
}

template<typename T, typename ...Args>
T max(T a, T b, Args... rest){
	if(a > b)
		return max(a, rest...);
	else
		return max(b, rest...);
}

template<typename T>
T clamp(T lo, T x, T hi){
	return min(max(lo, x), hi);
}

static_assert(sizeof(f32) == 4 && sizeof(f64) == 8, "Bad float size");
static_assert(sizeof(isize) == sizeof(usize), "Mismatched (i/u)size");
static_assert(sizeof(void(*)(void)) == sizeof(void*), "Function pointers and data pointers must be of the same width");
static_assert(sizeof(void(*)(void)) == sizeof(uintptr), "Mismatched pointer types");
static_assert(CHAR_BIT == 8, "Invalid char size");

//// Source Location ///////////////////////////////////////////////////////////
struct Source_Location {
    cstring filename;
    cstring caller_name;
    i32 line;
};

#define this_location() _this_location_0()

#define _this_location_0() (Source_Location){ \
    .filename = __FILE__, \
    .caller_name = __func__, \
    .line = __LINE__, \
}

//// Assert ////////////////////////////////////////////////////////////////////
// Crash if `pred` is false, this is disabled in non-debug builds
void debug_assert_ex(bool pred, cstring msg, Source_Location loc);

void bounds_check_assert_ex(bool pred, cstring msg, Source_Location loc);

#if defined(NDEBUG) || defined(RELEASE_MODE)
#define debug_assert(Pred, Msg) ((void)0)
#else
#define debug_assert(Pred, Msg) debug_assert_ex(Pred, Msg, this_location())
#endif

#if defined(DISABLE_BOUNDS_CHECK)
#define bounds_check_assert(Pred, Msg) ((void)0)
#else
#define bounds_check_assert(Pred, Msg) bounds_check_assert_ex(Pred, Msg, this_location())
#endif

// Crash if `pred` is false, this is always enabled
void panic_assert_ex(bool pred, cstring msg, Source_Location loc);

#define panic_assert(Pred, Msg) panic_assert_ex(Pred, Msg, this_location())

// Crash the program with a fatal error
[[noreturn]] void panic(cstring msg);

// Crash the program due to unimplemented code paths, this should *only* be used
// during development
[[noreturn]] void unimplemented();

//// Slices ////////////////////////////////////////////////////////////////////
template<typename T>
struct Slice {
	T* _data {nullptr};
	isize _length {0};

	isize size() const { return _length; }

	T* raw_data() const { return _data; }

	bool empty() const { return _length == 0 || _data == nullptr; }

	T& operator[](isize idx) noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	T const& operator[](isize idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	// Get a sub-slice in the interval a..slice.size()
	Slice<T> sub(isize from){
		bounds_check_assert(from >= 0 && from < _length, "Index to sub-slice is out of bounds");
		Slice<T> s;
		s._length = _length - from;
		s._data = &_data[from];
		return s;
	}

	// Get a sub-slice in the interval a..b (end exclusive)
	Slice<T> sub(isize from, isize to){
		bounds_check_assert(from <= to, "Improper slicing range");
		bounds_check_assert((from >= 0 && from < _length), "Slice index `from` is out of bounds");
		bounds_check_assert(to >= 0 && to <= _length, "Slice index `to` is out of bounds");

		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}

	Slice() : _data{nullptr}, _length{0} {}

	static Slice<T> from_pointer(T* ptr, isize len){
		Slice<T> s;
		s._data = ptr;
		s._length = len;
		return s;
	}
};

//// Atomic ////////////////////////////////////////////////////////////////////
// Boilerplate around C++'s standard library atomics, can't do much about that without leaking the entire std namespace
template<typename T>
using Atomic = std::atomic<T>;

using std::memory_order_relaxed,
    std::memory_order_consume,
    std::memory_order_acquire,
    std::memory_order_release,
    std::memory_order_acq_rel,
    std::memory_order_seq_cst;

using
	std::atomic_store,
	std::atomic_store_explicit,
	std::atomic_load,
	std::atomic_load_explicit,
	std::atomic_exchange,
	std::atomic_exchange_explicit,
	std::atomic_compare_exchange_weak,
	std::atomic_compare_exchange_weak_explicit,
	std::atomic_compare_exchange_strong,
	std::atomic_compare_exchange_strong_explicit,
	std::atomic_fetch_add,
	std::atomic_fetch_add_explicit,
	std::atomic_fetch_sub,
	std::atomic_fetch_sub_explicit,
	std::atomic_fetch_and,
	std::atomic_fetch_and_explicit,
	std::atomic_fetch_or,
	std::atomic_fetch_or_explicit,
	std::atomic_fetch_xor,
	std::atomic_fetch_xor_explicit;

//// Sync //////////////////////////////////////////////////////////////////////
namespace sync {
constexpr i32 SPINLOCK_UNLOCKED = 0;
constexpr i32 SPINLOCK_LOCKED = 1;

// The zeroed state of a spinlock is unlocked, to be effective across threads
// it's important to keep the spinlock outside of the stack and never mark it as
// a thread_local struct.
struct Spinlock {
	Atomic<i32> _state{SPINLOCK_UNLOCKED};

	// Enter a busy wait loop until spinlock is acquired(locked)
	void acquire();

	// Try to lock spinlock, if failed, just move on. Returns if lock was locked.
	bool try_acquire();

	// Release(unlock) the spinlock
	void release();
};
}
//// Memory ////////////////////////////////////////////////////////////////////
namespace mem {
enum class Allocator_Op : byte {
	Query    = 0, // Query allocator's capabilities
	Alloc    = 1, // Allocate a chunk of memory
	Resize   = 2, // Resize an allocation in-place
	Free     = 3, // Mark allocation as free
	Free_All = 4, // Mark allocations as free
};

enum class Allocator_Capability : u32 {
	Alloc_Any = 1 << 0, // Can alloc any size
	Free_Any  = 1 << 1, // Can free in any order
	Free_All  = 1 << 2, // Can free all allocations
	Resize    = 1 << 3, // Can resize in-place
	Align_Any = 1 << 4, // Can alloc aligned to any alignment
};

// Memory allocator method
using Allocator_Func = void* (*) (
	void* impl,
	Allocator_Op op,
	void* old_ptr,
	isize size,
    isize align,
	u32* capabilities
);

// Memory allocator interface
struct Allocator {
	void* _impl{0};
	Allocator_Func _func{0};

	// Get capabilities of allocator as a number, gets capability bit-set
	u32 query_capabilites();

	// Allocate fresh memory, filled with 0s. Returns NULL on failure.
	void* alloc(isize size, isize align);

	// Re-allocate memory in-place without changing the original pointer. Returns
	// NULL on failure.
	void* resize(void* ptr, isize new_size);

	// Free pointer to memory, includes alignment information, which is required for
	// some allocators, freeing NULL is a no-op
	void free_ex(void* p, isize size, isize align);

	// Free pointer to memory, freeing NULL is a no-op
	void free(void* p);

	// Free all pointers owned by allocator
	void free_all();

	// Re-allocate to new_size, first tries to resize in-place, then uses
	// alloc->copy->free to attempt reallocation, returns null on failure
	void* realloc(void* ptr, isize old_size, isize new_size, isize align);
};

// Set n bytes of p to value.
void set(void* p, byte val, isize nbytes);

// Copy n bytes for source to destination, they may overlap.
void copy(void* dest, void const * src, isize nbytes);

// Compare 2 buffers of memory, returns -1, 0, 1 depending on which buffer shows
// a bigger byte first, 0 meaning equality.
i32 compare(void const * a, void const * b, isize nbytes);

// Copy n bytes for source to destination, they should not overlap, this tends
// to be faster then mem_copy
void copy_no_overlap(void* dest, void const * src, isize nbytes);

// Align p to alignment a, this only works if a is a non-zero power of 2
uintptr align_forward_ptr(uintptr p, uintptr a);

// Align p to alignment a, this works for any positive non-zero alignment
uintptr align_forward_size(isize p, isize a);

} /* Namespace mem */

//// Make & Destroy ////////////////////////////////////////////////////////////
// Allocate one of object of a type using allocator
template<typename T>
T* make(mem::Allocator al){
	T* p = al.alloc(sizeof(T), alignof(T));
	if(p != nullptr){
		new (&p) T();
	}
	return p;
}

// Allocate slice of a type using allocator
template<typename T>
Slice<T> make(isize count, mem::Allocator al){
	T* p = al.alloc(sizeof(T) * count, alignof(T) * count);
	if(p != nullptr){
		for(isize i = 0; i < count; i ++){
			new (&p[i]) T();
		}
	}
	return Slice<T>::from_pointer(p, count);
}

// Deallocate object from allocator
template<typename T>
void destroy(T* ptr, mem::Allocator al){
	ptr->~T();
	al.free(ptr);
}

// Deallocate slice from allocator
template<typename T>
void destroy(Slice<T> s, mem::Allocator al){
	isize n = s.size();
	T* ptr = s.raw_data();
	for(isize i = 0; i < n; i ++){
		ptr[i]->~T();
	}
	al.free(ptr);
}

//// UTF-8 /////////////////////////////////////////////////////////////////////
namespace utf8 {
// UTF-8 encoding result, a len = 0 means an error.
struct Encode_Result {
	byte bytes[4];
	i8 len;
};

// UTF-8 encoding result, a len = 0 means an error.
struct Decode_Result {
	rune codepoint;
	i8 len;
};

// The error rune
constexpr rune ERROR = 0xfffd;

// The error rune but encoded
constexpr Encode_Result ERROR_ENCODED = {
	.bytes = {0xef, 0xbf, 0xbd},
	.len = 1, // Mainly for convenience when advancing over a series of invalid bytes.
};

// Encode a unicode codepoint
Encode_Result encode(rune c);

// Decode a codepoint from a UTF8 buffer of bytes
Decode_Result utf8_decode(Slice<byte> buf);

// Allows to iterate a stream of bytes as a sequence of runes
struct Iterator {
	Slice<byte> data;
	isize current;

	// Steps iterator forward and puts rune and length advanced into pointers,
	// returns false when finished.
	bool next(rune* r, i8* len);

	// Steps iterator backward and puts rune and its length into pointers,
	// returns false when finished.
	bool prev(rune* r, i8* len);

    // Steps iterator forward and returns the read rune. Returns 0 when finished
	rune next();

    // Steps iterator backward and returns the read rune. Returns 0 when finished
	rune prev();
};

}/* Namespace utf8 */

//// Strings ///////////////////////////////////////////////////////////////////
struct String {
	byte const * _data{nullptr};
	isize _length{0};

	// Size (in bytes)
	isize size() const;

	// Size (in codepoints)
	isize rune_count();

	// Create a substring
	String sub(isize from, isize to);
	
    // Create a substring
	String sub(isize from);

    // Gets the string data as a slice of bytes, this is not necessarily safe to modify as the string data
    // may live in the static section.
    Slice<byte> to_bytes_unsafe();

	// Create string from C-style string
	static String from_cstr(cstring data);

	// Create string from a piece of a C-style string
	static String from_cstr(cstring data, isize start, isize length);

	// Create string from a raw byte buffer
	static String from_bytes(Slice<byte>);

	// Get an utf8 iterator from string
	utf8::Iterator iterator();

	// Get an utf8 iterator from string, already at the end, to be used for reverse iteration
	utf8::Iterator iterator_reversed();

	// Check if 2 strings are equal
	bool operator==(String lhs) const {
		if(lhs._length != _length){ return false; }
		return mem::compare(_data, lhs._data, _length) == 0;
	}

    // Implicit construction, this is one of the very vew places it's going to be used
    // so we are able able to have String be initalized by a C string literal.
    String(){}
    String(cstring cstr);
};

// Length of cstring
isize cstring_len(cstring cstr);


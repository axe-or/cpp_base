#pragma once

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

template<typename T>
using Atomic = std::atomic<T>;

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

//// Defer /////////////////////////////////////////////////////////////////////
namespace impl_defer {
	template<typename F>
	struct Deferred {
		F f;
		explicit Deferred(F&& f) : f(static_cast<F&&>(f)){}
		~Deferred(){ f(); }
	};

	template<typename F>
	auto make_deferred(F&& f){
		return Deferred<F>(static_cast<F&&>(f));
	}
}

#define _defer_impl_glue0(X, Y) X##Y
#define _defer_impl_glue1(X, Y) _defer_impl_glue0(X, Y)
#define _defer_impl_glue_num(X) _defer_impl_glue1(X, __COUNTER__)

#define defer(Stmt) auto _defer_impl_glue_num(_defer_fn_) = ::impl_defer::make_deferred([&](){ Stmt ; });

//// Source Location ///////////////////////////////////////////////////////////
typedef struct Source_Location Source_Location;

struct Source_Location {
    cstring filename;
    cstring caller_name;
    i32 line;
};

#define this_location() this_location_()

#define this_location_() (Source_Location){ \
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
void ensure_ex(bool pred, cstring msg, Source_Location loc);

#define ensure(Pred, Msg) ensure_ex(Pred, Msg, this_location())

// Crash the program with a fatal error
[[noreturn]] void panic(cstring msg);

// Crash the program due to unimplemented code paths, this should *only* be used
// during development
[[noreturn]] void unimplemented();

//// Slices ////////////////////////////////////////////////////////////////////
template<typename T>
struct Slice {
	T* _data;
	isize _length;

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
	Slice<T> slice(isize from){
		bounds_check_assert(from >= 0 && from < _length, "Index to sub-slice is out of bounds");
		Slice<T> s;
		s._length = _length - from;
		s._data = &_data[from];
		return s;
	}

	// Get a sub-slice in the interval a..b (end exclusive)
	Slice<T> slice(isize from, isize to){
		bounds_check_assert(from <= to, "Improper slicing range");
		bounds_check_assert(from >= 0 && from < _length, "Index to sub-slice is out of bounds");
		bounds_check_assert(to >= 0 && to <= _length, "Index to sub-slice is out of bounds");

		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}

	Slice() : _data{nullptr}, _length{0} {}

	bool equals(Slice<T> s) {
		if(_length != s._length){ return false; }
		for(isize i = 0; i < s.size(); i ++){
			if(s._data[i] != _data[i]){ return false; }
		}
		return true;
	}

	static Slice<T> from_pointer(T* ptr, isize len){
		Slice<T> s;
		s._data = ptr;
		s._length = len;
		return s;
	}
};

//// Fixed Array ///////////////////////////////////////////////////////////////
// Basically the same interface as a dynamic array, but with a fixed backing buffer
template<typename T, isize N>
struct Fixed_Array {
	T _data[N];
	isize _length = 0;

	isize size() const { return _length; }

	isize cap() const { return N; }

	T& operator[](isize idx){
		bounds_check_assert(idx >= 0 && idx <_length, "Out of bounds index on fixed array");
		return _data[idx];
	}

	T const & operator[](isize idx) const {
		bounds_check_assert(idx >= 0 && idx <_length, "Out of bounds index on fixed array");
		return _data[idx];
	}

	bool append(T val){
		if(_length >= N){
			return false;
		}
		_data[_length] = val;
		_length += 1;
		return true;
	}

	Slice<T> slice(){
		return Slice<T>::from_pointer(&_data[0], N);
	}
};

//// Memory ////////////////////////////////////////////////////////////////////
namespace mem {
enum class Allocator_Op : byte {
	Query    = 0, // Query allocator's capabilities
	Alloc    = 1, // Allocate a chunk of memory
	Resize   = 2, // Resize an allocation in-place
	Free     = 3, // Mark allocation as free
	Free_All = 4, // Mark allocations as free
	// Realloc  = 5, // Re-allocate pointer
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
	u32 query_capabilites(){
		u32 n = 0;
		_func(_impl, Allocator_Op::Query, nullptr, 0, 0, &n);
		return n;
	}

	// Allocate fresh memory, filled with 0s. Returns NULL on failure.
	void* alloc(isize size, isize align){
		return _func(_impl, Allocator_Op::Alloc, nullptr, size, align, nullptr);
	}

	// Re-allocate memory in-place without changing the original pointer. Returns
	// NULL on failure.
	void* resize(void* ptr, isize new_size){
		return _func(_impl, Allocator_Op::Resize, ptr, new_size, 0, nullptr);
	}

	// Free pointer to memory, includes alignment information, which is required for
	// some allocators, freeing NULL is a no-op
	void free_ex(void* ptr, isize size, isize align){
		_func(_impl, Allocator_Op::Free, ptr, size, align, nullptr);
	}

	// Free pointer to memory, freeing NULL is a no-op
	void free(void* ptr){
		_func(_impl, Allocator_Op::Free, ptr, 0, 0, nullptr);
	}

	// Free all pointers owned by allocator
	void free_all(){
		_func(_impl, Allocator_Op::Free_All, nullptr, 0, 0, nullptr);
	}

	// Re-allocate, unlike resize(), the pointer may change, returns null on
	// failure, if allocation is moved somewhere else, the previous pointer
	// is freed.
	// void* realloc(void* ptr, isize new_size, isize align){
	// 	return _func(_impl, Allocator_Op::Realloc, ptr, new_size, align, nullptr);
	// }
};

// Set n bytes of p to value.
void set(void* p, byte val, isize nbytes);

// Copy n bytes for source to destination, they may overlap.
void copy(void* dest, void const * src, isize nbytes);

// Compare 2 buffers of memory, returns -1, 0, 1 depending on which buffer shows
// a bigger byte first, 0 meaning equality.
i32 compare(void const * a, void const * b, isize nbytes);

// Copy n bytes for source to destination, they should not overlap, this tends
// to be faster then copy
void copy_no_overlap(void* dest, void const * src, isize nbytes);

// Align p to alignment a, this only works if a is a non-zero power of 2
uintptr align_forward_ptr(uintptr p, uintptr a);

// Align p to alignment a, this works for any positive non-zero alignment
uintptr align_forward_size(isize p, isize a);
} /* Namespace mem */

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

// The error rune, byte encoded
constexpr Encode_Result ERROR_ENCODED = {
	.bytes = {0xef, 0xbf, 0xbd},
	.len = 0,
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

	// Steps iterator forward and returns rune, returns 0 on end.
	rune next();

	// Steps iterator backward and returns rune, returns 0 when finished.
	rune prev();
};
} /* Namespace utf8 */

//// Strings ///////////////////////////////////////////////////////////////////
static inline
isize cstring_len(cstring cstr){
	constexpr isize CSTR_MAX_LENGTH = (~(u32)0) >> 1;
	isize size = 0;
	for(isize i = 0; i < CSTR_MAX_LENGTH && cstr[i] != 0; i += 1){
		size += 1;
	}
	return size;
}

struct String {
	byte const * _data = nullptr;
	isize _length = 0;

	// Size (in bytes)
	isize size() const { return _length; }

	// Size (in codepoints)
	isize rune_count();

	// Get a sub-string in the byte interval a..b (end exclusive)
	String sub(isize from, isize to);

	// Get an utf8 iterator from string
	utf8::Iterator iterator();

	// Get an utf8 iterator from string, already at the end, to be used for reverse iteration
	utf8::Iterator iterator_reversed();

	// Trim leading and trailing runes from cutset
	String trim(String cutset);
	
	// Trim leading runes from cutset
	String trim_leading(String cutset);

	// Trim trailing from cutset
	String trim_trailing(String cutset);

	// Create string from C-style string
	static String from_cstr(cstring data){
		String s;
		s._data = (byte const*)data;
		s._length = cstring_len(data);
		return s;
	}

	// Create string from a piece of a C-style string
	static String from_cstr(cstring data, isize start, isize length){
		String s;
		s._data = (byte const*)&data[start];
		s._length = length;
		return s;
	}

	// Create string from a raw byte buffer
	static String from_bytes(Slice<byte> buf){
		String s;
		s._data = buf.raw_data();
		s._length = buf.size();
		return s;
	}

	// Implict conversion, this is one of the very vew places an implicit
	// conversion is made in the library, mostly to write C-strings more
	// ergonomic.
	String(){}
	String(cstring s) : _data((byte const*)s), _length(cstring_len(s)){}

	// Get byte at position
	byte operator[](isize idx) const {
		bounds_check_assert(idx >= 0 && idx <_length, "Out of bounds index on string");
		return _data[idx];
	}

	// Check if 2 strings are equal
	bool operator==(String lhs) const {
		if(lhs._length != _length){ return false; }
		return mem::compare(_data, lhs._data, _length) == 0;
	}
};

//// Arena Allocator ///////////////////////////////////////////////////////////
namespace mem {
struct Arena {
	isize offset;
	isize capacity;
	uintptr last_allocation;
	byte* data;

	// Resize arena allocation in-place, gives back same pointer on success, null on failure
	void* resize(void* ptr, isize new_size);

	// Reset arena, marking all its owned pointers as freed
	void free_all();

	// Allocate `size` bytes aligned to `align`, return null on failure
	void* alloc(isize size, isize align);

	// Get arena as a conforming instance to the allocator interface
	Allocator allocator();

	// Initialize a memory arena with a buffer
	static Arena from_buffer(Slice<byte> s);
};

} /* Namespace mem */

//// Heap Allocator ////////////////////////////////////////////////////////////
namespace mem {
// Just a wrapper around aligned_alloc and free
Allocator heap_allocator();
} /* Namespace mem */


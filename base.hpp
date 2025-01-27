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
#define defer(Stmt) [[maybe_unused]] auto _defer_impl_glue_num(_defer_fn_) = ::impl_defer::make_deferred([&](){ Stmt ; });

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

#define ensure(Pred, Msg) ensure_ex((Pred), Msg, this_location())

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

	// Length of slice (in elements)
	isize size() const { return _length; }

	// Length of slice (in bytes)
	isize raw_size() const { return _length * sizeof(T); }

	// Raw pointer to slice's backing memory
	T* raw_data() const { return _data; }

	// Is the slice empty?
	bool empty() const { return _length == 0 || _data == nullptr; }

	T& operator[](isize idx) noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	T const& operator[](isize idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	// Find first occurance of item, this involves a linear scan. Returns -1 if not found
	isize find(T e){
		for(isize i = 0; i < _length; i++){
			if(_data[i] == e){ return i; }
		}
		return -1;
	}

	// Identity function, for consistency with other contigous array types
	Slice<T> slice(){
		return *this;
	}

	// Get the sub-slice of elements after index (inclusive)
	Slice<T> slice_right(isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		Slice<T> s;
		s._length = _length - idx;
		s._data = &_data[idx];
		return s;
	}

	// Get the sub-slice of elements before index (exclusive)
	Slice<T> slice_left(isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		Slice<T> s;
		s._length = idx;
		s._data = _data;
		return s;
	}

	// Get the sub-slice of interval a..b (end exclusive)
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

//// Option ////////////////////////////////////////////////////////////////////
template<typename T>
struct Option {
	T _inner;
	bool _has_value = false;

	// Safe to get value
	bool ok() const { return _has_value; }

	// Get the value, panics otherwhise
	T get(){
		if(_has_value){
			return _inner;
		}
		panic("Attempt to get() empty option");
	}

	// Get the value, if nil, use a default one
	T get_or(T alt){
		if(_has_value){
			return _inner;
		}
		return alt;
	}

	// Get the value, regardless if initialized
	T get_unchecked(){
		return _inner;
	}

	// Destroy inner value, if any
	void clear(){
		if(_has_value){
			_inner.~T();
			_has_value = false;
		}
	}

	// Implict constructors
	Option(){}
	Option(T v) : _inner(v), _has_value(true) {}

	static Option<T> empty(){
		return Option<T>();
	}

	static Option<T> from(T v){
		return Option<T>(v);
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
	void free(void* ptr, isize size, isize align){
		_func(_impl, Allocator_Op::Free, ptr, size, align, nullptr);
	}

	// Free all pointers owned by allocator
	void free_all(){
		_func(_impl, Allocator_Op::Free_All, nullptr, 0, 0, nullptr);
	}

	// Helper to create any type
	template<typename T>
	T* make(){
		auto p = (T*)this->alloc(sizeof(T), alignof(T));
		if(p != nullptr){
			new (p) T();
		}
		return p;
	}

	// Helper to create any type
	template<typename T>
	Slice<T> make(isize n){
		auto p = (T*)this->alloc(n * sizeof(T), alignof(T));
		if(p != nullptr){
			for(isize i = 0; i < n; i++){
				new (&p[i]) T();
			}
		}
		return Slice<T>::from_pointer(p, n);
	}

	// Helper to deallocate any trivial type
	template<typename T>
	void destroy(T* p){
		this->free((void*)p, sizeof(T), alignof(T));
	}

	// Helper to deallocate any trivial type
	template<typename T>
	void destroy(Slice<T> s){
		T* buf = s.raw_data();
		isize n = s.size();
		this->free((void*)buf, n * sizeof(T), alignof(T));
	}

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
	isize rune_count() const;

	// Get a sub-string in the byte interval a..b (end exclusive)
	String sub(isize from, isize to) const;

	// Get an utf8 iterator from string
	utf8::Iterator iterator() const;

	// Get an utf8 iterator from string, already at the end, to be used for reverse iteration
	utf8::Iterator iterator_reversed() const;

	// Trim leading and trailing runes from cutset
	String trim(String cutset) const;
	
	// Trim leading runes from cutset
	String trim_leading(String cutset) const;

	// Trim trailing from cutset
	String trim_trailing(String cutset) const;

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
	byte operator[](isize idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx <_length, "Out of bounds index on string");
		return _data[idx];
	}

	// Check if 2 strings are equal
	bool operator==(String lhs) const noexcept {
		if(lhs._length != _length){ return false; }
		return mem::compare(_data, lhs._data, _length) == 0;
	}

	// Check if 2 strings are different
	bool operator!=(String lhs) const noexcept {
		if(lhs._length != _length){ return false; }
		return mem::compare(_data, lhs._data, _length) != 0;
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

//// Dynamic Array /////////////////////////////////////////////////////////////
static constexpr isize dynamic_array_default_capacity = 16;

template<typename T>
struct Dynamic_Array {
	T* data = nullptr;
	isize length = 0;
	isize capacity = 0;
	mem::Allocator allocator {0};

	// Size of array (in elements)
	isize size() const { return length; }

	// Current capacity (in elements)
	isize cap() const { return capacity; }

	// Resize array's underlying buffer, returns if succeeded
	bool resize(isize new_cap){
		T* new_data = nullptr;

		new_data = (T*)allocator.resize(data, new_cap * sizeof(T));

		// Needs new allocation
		if(new_data == nullptr){
			new_data = (T*)allocator.alloc(new_cap * sizeof(T), alignof(T));
			if(new_data == nullptr){ return false; }

			mem::copy_no_overlap(new_data, data, sizeof(T) * min(new_cap, length));
			allocator.free(data, sizeof(T) * capacity, alignof(T));
		}

		capacity = new_cap;
		length   = min(length, new_cap);
		data     = new_data;
		return true;
	}

	// Add element to end of array, returns if succeeded
	bool append(T e){
		if(length >= capacity){
			isize new_cap = max(capacity * 2, dynamic_array_default_capacity);
			if(!this->resize(new_cap)){ return false; }
		}

		data[length] = e;
		length += 1;
		return true;
	}

	// Reset the array's length, keeps its capacity
	void clear(){
		length = 0;
	}

	// Remove element at index, keeping the order
	void remove_ordered(isize idx){
		bounds_check_assert(idx >= 0 && idx < length, "Out of bounds remove index");
		mem::copy(&data[idx], &data[idx + 1], (length - idx) * sizeof(T));
		length -= 1;
	}

	// Remove element at index, does not keep element order
	void remove_unordered(isize idx){
		bounds_check_assert(idx >= 0 && idx < length, "Out of bounds remove index");
		data[idx] = data[length - 1];
		length -= 1;
	}

	// Insert element at index, keeps elements in order
	bool insert_ordered(isize idx, T e){
		bounds_check_assert(idx >= 0 && idx <= length, "Out of bounds insertion index");
		if(idx == length){
			return append(e);
		}

		if(length >= capacity){
			isize new_cap = max(capacity * 2, dynamic_array_default_capacity);
			if(!this->resize(new_cap)){ return false; }
		}

		mem::copy(&data[idx + 1], &data[idx], (length - idx) * sizeof(T));
		data[idx] = e;
		length += 1;
		return true;
	}

	// Insert element at index, does not keep elements in order
	bool insert_unordered(isize idx, T e){
		bounds_check_assert(idx >= 0 && idx <= length, "Out of bounds insertion index");
		if(idx == length){
			return append(e);
		}

		if(length >= capacity){
			isize new_cap = max(capacity * 2, dynamic_array_default_capacity);
			if(!this->resize(new_cap)){ return false; }
		}

		data[length] = data[idx];
		data[idx] = e;
		length += 1;
		return true;
	}

	// Pop element form end of the array, returns if element was popped
	bool pop(){
		if(length > 0){
			length -= 1;
			return true;
		}
		return false;
	}

	// Get current array data as a slice
	Slice<T> slice(){
		return Slice<T>::from_pointer(data, length);
	}

	// Get the sub-slice of elements after index (inclusive)
	Slice<T> slice_right(isize idx){
		bounds_check_assert(idx >= 0 && idx < length, "Index to sub-slice is out of bounds");
		return Slice<T>::from_pointer(&data[idx], length - idx);
	}

	// Get the sub-slice of elements before index (exclusive)
	Slice<T> slice_left(isize idx){
		bounds_check_assert(idx >= 0 && idx < length, "Index to sub-slice is out of bounds");
		return Slice<T>::from_pointer(data, idx);
	}

	// Get the sub-slice of interval a..b (end exclusive)
	Slice<T> slice(isize from, isize to){
		bounds_check_assert(from <= to, "Improper slicing range");
		bounds_check_assert(from >= 0 && from < length, "Index to sub-slice is out of bounds");
		bounds_check_assert(to >= 0 && to <= length, "Index to sub-slice is out of bounds");
		return Slice<T>::from_pointer(&data[from], to - from);
	}

	T& operator[](isize idx) noexcept {
		bounds_check_assert(idx >= 0 && idx < length, "Index to dyanamic array is out of bounds");
		return data[idx];
	}

	T const& operator[](isize idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx < length, "Index to dyanamic array is out of bounds");
		return data[idx];
	}

	static Dynamic_Array<T> from(mem::Allocator allocator){
		Dynamic_Array<T> arr;
		arr.allocator = allocator;
		arr.resize(dynamic_array_default_capacity);
		return arr;
	}
};



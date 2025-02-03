#ifndef _base_hpp_include_
#define _base_hpp_include_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <tgmath.h>
#include <limits.h>
#include <float.h>

#include <atomic>

using I8  = int8_t;
using I16 = int16_t;
using I32 = int32_t;
using I64 = int64_t;

using U8  = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;

using uint = unsigned int;
using Byte = uint8_t;
using Rune = I32;

using Size = ptrdiff_t;

using Uintptr = uintptr_t;

using F32 = float;
using F64 = double;

constexpr Size cache_line_size = 64;

template<typename T>
using Atomic = std::atomic<T>;

template<typename A, typename B = A>
struct Pair {
	A a;
	B b;
};

// Swap bytes around, useful for when dealing with endianess
template<typename T>
void swap_bytes(T* data){
	Size len = sizeof(T);
	for(Size i = 0; i < (len / 2); i += 1){
		Byte temp = data[i];
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

static_assert(sizeof(F32) == 4 && sizeof(F64) == 8, "Bad float size");
static_assert(sizeof(Size) == sizeof(Size), "Mismatched (i/u)size");
static_assert(sizeof(void(*)(void)) == sizeof(void*), "Function pointers and data pointers must be of the same width");
static_assert(sizeof(void(*)(void)) == sizeof(Uintptr), "Mismatched pointer types");
static_assert(CHAR_BIT == 8, "Invalid char size");

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

[[noreturn]]
void panic(char const * msg);

void debug_assert(bool pred, char const * msg);

void ensure(bool pred, char const * msg);

void bounds_check_assert(bool pred, char const * msg);

template<typename T>
struct Slice {
private:
	T*   _data   = nullptr;
	Size _length = 0;
public:
	T& operator[](Size idx) noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	T const& operator[](Size idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	// Identity function, for consistency with other contigous array types
	Slice<T> slice(){
		return *this;
	}

	// Get the sub-slice of elements after index (inclusive)
	Slice<T> slice_right(Size idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		Slice<T> s;
		s._length = _length - idx;
		s._data = &_data[idx];
		return s;
	}

	// Get the sub-slice of elements before index (exclusive)
	Slice<T> slice_left(Size idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		Slice<T> s;
		s._length = idx;
		s._data = _data;
		return s;
	}

	// Get the sub-slice of interval a..b (end exclusive)
	Slice<T> slice(Size from, Size to){
		bounds_check_assert(
			from >= 0 && from < _length &&
			to >= 0 && to <= _length &&
			from <= to,
			"Index to sub-slice is out of bounds");
		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}

	static Slice<T> from_pointer(T* data, Size len){
		bounds_check_assert(len > 0, "Negative length value");
		Slice<T> s;
		s._data = data;
		s._length = len;
		return s;
	}

	Size len() const { return _length; }

	T* raw_data() const { return _data; }

	bool empty() const { return _length == 0 || _data == nullptr; }
};


namespace mem {
void set(void* p, Byte val, Size nbytes);

void copy(void* dest, void const * src, Size nbytes);

void copy_no_overlap(void* dest, void const * src, Size nbytes);

I32 compare(void const * a, void const * b, Size nbytes);

Uintptr align_forward(Uintptr p, Uintptr a);

Uintptr align_forward(Size p, Size a);

bool valid_alignment(Size a);

namespace virt {
	struct PageBlock {
		Size reserved;
		Size commited;
		void* pointer;

		static PageBlock make(Size nbytes);

		void destroy();

		void* push(Size nbytes);

		void pop(Size nbytes);
	};


	void* reserve(Size nbytes);

	void release(void* pointer);

	void* commit(void* pointer, Size nbytes);

	void decommit(void* pointer, Size nbytes);
};

enum struct ArenaType : U32 {
	Buffer = 0,
	Virtual,
};

struct Arena {
	ArenaType type;
	Size offset;
};

};

namespace utf8 {
struct EncodeResult {
	Byte bytes[4];
	I32 len;
};

struct DecodeResult {
	Rune codepoint;
	I32 len;
};

constexpr Rune ERROR = 0xfffd;

// The error rune, Byte encoded
constexpr EncodeResult ERROR_ENCODED = {
	.bytes = {0xef, 0xbf, 0xbd},
	.len = 0,
};

EncodeResult encode(Rune c);

DecodeResult utf8_decode(Slice<Byte> buf);

struct Iterator {
	Slice<Byte> data;
	Size current;

	bool next(Rune* r, I32* len);

	bool prev(Rune* r, I32* len);

	Rune next();

	Rune prev();
};
} /* Namespace utf8 */

#endif /* Include guard */

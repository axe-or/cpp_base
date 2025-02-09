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

namespace meta {
template <typename A, typename B>
inline constexpr bool same_type = false;

template <typename A>
inline constexpr bool same_type<A, A> = true;

template <typename A, typename B>
concept SameAs = same_type<A, B>;

template<class T> T&& decl_value() noexcept;

template<typename From, typename To>
concept ConvertibleTo = requires { static_cast<To>(decl_value<From>()); };
}

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

#define _impl_defer_concat0(X, Y) X##Y
#define _impl_defer_concat1(X, Y) _impl_defer_concat0(X, Y)
#define _impl_defer_concat_counter(X) _impl_defer_concat1(X, __COUNTER__)
#define defer(Stmt) auto _impl_defer_concat_counter(_defer_) = ::impl_defer::make_deferred([&](){ do { Stmt ; } while(0); return; })
}

[[noreturn]]
void panic(char const * msg);

void debug_assert(bool pred, char const * msg);

void ensure(bool pred, char const * msg);

void bounds_check_assert(bool pred, char const * msg);

template<typename T>
struct Slice {
private:
	T*   data   = nullptr;
	Size length = 0;
public:
	T& operator[](Size idx) noexcept {
		bounds_check_assert(idx >= 0 && idx < length, "Index to slice is out of bounds");
		return data[idx];
	}

	T const& operator[](Size idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx < length, "Index to slice is out of bounds");
		return data[idx];
	}

	Slice<T> operator[](Pair<Size> range){
		Size from = range.a;
		Size to = range.b;

		bounds_check_assert(from >= 0 && from < length && to >= 0 && to <= length && from <= to, "Index to sub-slice is out of bounds");

		Slice<T> s;
		s.length = to - from;
		s.data = &data[from];
		return s;
	}

	Slice(){}

	explicit Slice(T* data, Size len) : data{data}, length{len} {}

	// Accessors
	friend Size len(Slice<T> s){ return s.length; }
	friend T* raw_data(Slice<T> s){ return s.data; }
};

template<typename T>
Slice<T> slice(T* data, Size len){
	bounds_check_assert(len > 0, "Negative length value");
	auto s = Slice<T>(data, len);
	return s;
}

// Get the sub-slice of interval a..b (end exclusive)
template<typename T>
Slice<T> slice(Slice<T> s, Size from, Size to){
	bounds_check_assert(
		from >= 0 && from < s.length &&
		to >= 0 && to <= s.length &&
		from <= to,
		"Index to sub-slice is out of bounds");
	Slice<T> res;
	res.length = to - from;
	res.data = &s.data[from];
	return res;
}

// Get the sub-slice of elements after index (inclusive)
template<typename T>
Slice<T> slice_right(Slice<T> s, Size idx){
	bounds_check_assert(idx >= 0 && idx < len(s), "Index to sub-slice is out of bounds");
	auto res = Slice<T>(&raw_data(s)[idx], len(s) - idx);
	return res;
}

// Get the sub-slice of elements before index (exclusive)
template<typename T>
Slice<T> slice_left(Slice<T> s, Size idx){
	bounds_check_assert(idx >= 0 && idx < len(s), "Index to sub-slice is out of bounds");
	auto res = Slice(raw_data(s), idx);
	return res;
}

constexpr Size mem_KiB = 1024ll;
constexpr Size mem_MiB = 1024ll * 1024ll;
constexpr Size mem_GiB = 1024ll * 1024ll * 1024ll;

void mem_set(void* p, Byte val, Size nbytes);

void mem_copy(void* dest, void const * src, Size nbytes);

void mem_copy_no_overlap(void* dest, void const * src, Size nbytes);

I32 mem_compare(void const * a, void const * b, Size nbytes);

static inline
bool mem_valid_alignment(Size a){
	return ((a & (a - 1)) == 0) && (a > 0);
}

static inline
Uintptr mem_align_forward_ptr(Uintptr p, Uintptr a){
	debug_assert(mem_valid_alignment(a), "Invalid memory alignment");
	Uintptr mod = p & (a - 1); // Fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

static inline
Size mem_align_forward_size(Size p, Size a){
	debug_assert(mem_valid_alignment(a), "Invalid size alignment");
	Size mod = p & (a - 1); // Fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

//// UTF-8 ////////////////////////////////////////////////////////////////////
struct Utf8EncodeResult {
	Byte bytes[4];
	I32 len;
};

struct Utf8DecodeResult {
	Rune codepoint;
	I32 len;
};

constexpr Rune ERROR = 0xfffd;

constexpr Utf8EncodeResult ERROR_ENCODED = {
	.bytes = {0xef, 0xbf, 0xbd},
	.len = 0,
};

Utf8EncodeResult utf8_encode(Rune c);

Utf8DecodeResult utf8_decode(Slice<Byte> buf);

struct Utf8Iterator {
	Slice<Byte> data;
	Size current;
};

bool iter_next(Utf8Iterator* it, Rune* r, I32* len);

bool iter_prev(Utf8Iterator* it, Rune* r, I32* len);

Rune iter_next(Utf8Iterator* it);

Rune iter_prev(Utf8Iterator* it);

//// String ///////////////////////////////////////////////////////////////////
static inline
Size cstring_len(char const* cstr){
	Size size = 0;
	for(Size i = 0; cstr[i] != 0; i += 1){
		size += 1;
	}
	return size;
}

struct String {
private:
	Byte const * _data = nullptr;
	Size _length = 0;

public:

	// Implict conversion, this is one of the very vew places an implicit
	// conversion is made in the library, mostly to write C-strings more
	// ergonomically.
	String(){}
	String(const char* cs) : _data{(Byte const*)cs}, _length{cstring_len(cs)}{}
	explicit String(Byte const* p, Size n) : _data{p}, _length{n}{}

	Byte operator[](Size idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx <_length, "Out of bounds index on string");
		return _data[idx];
	}

	String operator[](Pair<Size> range) const noexcept {
		Size from = range.a;
		Size to = range.b;
		bounds_check_assert(from >= 0 && from < _length && to >= 0 && to <= _length && from <= to, "Index to sub-string is out of bounds");

		return String(&_data[from], to - from);
	}

	bool operator==(String lhs) const noexcept {
		if(lhs._length != _length){ return false; }
		return mem_compare(_data, lhs._data, _length) == 0;
	}

	bool operator!=(String lhs) const noexcept {
		if(lhs._length != _length){ return false; }
		return mem_compare(_data, lhs._data, _length) != 0;
	}

	// Accessors
	friend Size len(String s){
		return s._length;
	}
	friend Byte* raw_data(String s){
		return (Byte*)s._data;
	}
};

static inline
String string_from_cstring(char const* data){
	return String((Byte const*)data, cstring_len(data));
}

static inline
String string_from_cstring(char const* data, Size start, Size length){
	return String((Byte const*) &data[start], length);
}

static inline
String string_from_bytes(Slice<Byte> buf){
	return String(raw_data(buf), len(buf));
}

static inline
String slice_right(String s, Size idx){
	bounds_check_assert(idx >= 0 && idx < len(s), "Index to sub-slice is out of bounds");
	auto res = String(&raw_data(s)[idx], len(s) - idx);
	return res;
}

static inline
String slice_left(String s, Size idx){
	bounds_check_assert(idx >= 0 && idx < len(s), "Index to sub-slice is out of bounds");
	auto res = String(raw_data(s), idx);
	return res;
}

Utf8Iterator str_iterator(String s);

Utf8Iterator str_iterator_reversed(String s);


//// Memory ///////////////////////////////////////////////////////////////////
constexpr U32 mem_protection_none    = 0;
constexpr U32 mem_protection_read    = (1 << 0);
constexpr U32 mem_protection_write   = (1 << 1);
constexpr U32 mem_protection_execute = (1 << 2);

enum class AllocatorOp : U32 {
	Query    = 0, // Query allocator's capabilities
	Alloc    = 1, // Allocate a chunk of memory
	Resize   = 2, // Resize an allocation in-place
	Free     = 3, // Mark allocation as free
	FreeAll  = 4, // Mark allocations as free
	Realloc  = 5, // Re-allocate pointer
};

enum class AllocatorCapability : U32 {
	AllocAny = 1 << 0, // Can alloc any size
	FreeAny  = 1 << 1, // Can free in any order
	FreeAll  = 1 << 2, // Can free all allocations
	Resize   = 1 << 3, // Can resize in-place
	AlignAny = 1 << 4, // Can alloc aligned to any alignment
	Virtual  = 1 << 5, // Can manipulate virtual memory
};

// Memory allocator method
using AllocatorFunc = void* (*) (
	void* impl,
	AllocatorOp op,
	void* old_ptr,
	Size old_size,
	Size size,
	Size align,
	U32* capabilities
);

// Memory allocator interface
struct Allocator {
	void* data = 0;
	AllocatorFunc func = 0;
};

void* mem_alloc(Allocator a, Size nbytes, Size align);

void* mem_resize(Allocator a, void* ptr, Size new_size);

void mem_free(Allocator a, void* ptr, Size old_size);

void* mem_realloc(Allocator a, void* ptr, Size old_size, Size new_size, Size align);

void mem_free_all(Allocator a);

template<typename T> [[nodiscard]]
T* make(Allocator a){
	T* p = (T*)mem_alloc(a, sizeof(T), alignof(T));
	return p;
}

template<typename T> [[nodiscard]]
Slice<T> make(Allocator a, Size elems){
	T* p = (T*)mem_alloc(a, sizeof(T) * elems, alignof(T));
	return slice<T>(p, elems);
}

template<typename T>
void destroy(Allocator a, T* obj){
	mem_free(a, obj, sizeof(T));
}

template<typename T>
void destroy(Allocator a, Slice<T> s){
	mem_free(a, raw_data(s), sizeof(T) * len(s));
}

template<typename T>
void destroy(Allocator a, String s){
	mem_free(a, raw_data(s), sizeof(T) * len(s));
}

//// Virtual Memory ///////////////////////////////////////////////////////////
constexpr Size mem_page_size = 4096;

struct PageBlock {
	Size reserved;
	Size commited;
	void* pointer;
};

PageBlock page_block_create(Size nbytes);

void page_block_destroy(PageBlock* blk);

void* page_block_push(PageBlock* blk, Size nbytes);

void page_block_pop(PageBlock* blk, Size nbytes);

void* virtual_reserve(Size nbytes);

void virtual_release(void* pointer, Size nbytes);

bool virtual_protect(void* pointer, U32 prot);

void* virtual_commit(void* pointer, Size nbytes);

void virtual_decommit(void* pointer, Size nbytes);


//// Arena ////////////////////////////////////////////////////////////////////
enum struct ArenaType : U32 {
	Buffer  = 0,
	Virtual = 1,
};

struct Arena {
	PageBlock data;
	Size offset;
	Uintptr last_allocation;
	ArenaType type;
};

void* arena_alloc(Arena* a, Size nbytes, Size align);

void* arena_resize_in_place(Arena* a, void* ptr, Size new_size);

void* arena_realloc(Arena* a, void* ptr, Size old_size, Size new_size, Size align);

void arena_free_all(Arena* a);

Arena arena_from_buffer(Slice<U8> buf);

Arena arena_create_virtual(Size reserve);

void arena_destroy(Arena* a);

Allocator arena_allocator(Arena* a);

//// Dynamic Array ////////////////////////////////////////////////////////////
template<typename T>
struct DynamicArray {
	T*        data;
	Size      capacity;
	Size      length;
	Allocator allocator;

	T& operator[](Size idx){
		bounds_check_assert(idx >= 0 && idx < length, "Out of bounds access to dynamic array");
		return data[idx];
	}

	T const& operator[](Size idx) const{
		bounds_check_assert(idx >= 0 && idx < length, "Out of bounds access to dynamic array");
		return data[idx];
	}

	// Accessors
	friend Size len(DynamicArray<T> a){ return a.length; }
	friend Size cap(DynamicArray<T> a){ return a.capacity; }
	friend T* raw_data(DynamicArray<T> a){ return a.data; }
};

template<typename T>
void clear(DynamicArray<T>* arr){
	mem_free(arr->allocator, arr->data, arr->length);
}

template<typename T>
Slice<T> slice(DynamicArray<T> arr){
	return slice(arr.data, arr.length);
}

template<typename T>
DynamicArray<T> dynamic_array_create(Allocator alloc, Size initial_cap = 16){
	DynamicArray<T> arr;
	arr.capacity = initial_cap;
	arr.length = 0;
	arr.allocator = alloc;
	arr.data = (T*)mem_alloc(alloc, initial_cap * sizeof(T), alignof(T));
	return arr;
}

template<typename T, typename U = T>
bool append(DynamicArray<T>* arr, U elem){
	[[unlikely]] if(arr->length >= arr->capacity){
		Size new_cap = mem_align_forward_size(16, arr->length * 2);
		auto new_data = (T*)mem_realloc(
				arr->allocator,
				arr->data,
				arr->capacity * sizeof(T),
				new_cap, alignof(T));

		if(new_data == nullptr){
			return false;
		}
		arr->data = new_data;
	}

	arr->data[arr->length] = T(elem);
	arr->length += 1;
	return true;
}

template<typename T>
bool pop(DynamicArray<T>& arr){
	if(arr.length <= 0){ return false; }
	arr->len -= 1;
	return true;
}


//// String Utilities /////////////////////////////////////////////////////////
String str_trim(String s, String cutset);

String str_trim_leading(String s, String cutset);

String str_trim_trailing(String s, String cutset);

Size str_rune_count(String s);

bool str_starts_with(String s, String prefix);

bool str_ends_with(String s, String suffix);

Size str_find(String s, String substr, Size start = 0);

[[nodiscard]]
String str_clone(String s, Allocator allocator);

#endif /* Include guard */

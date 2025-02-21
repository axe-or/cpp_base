#ifndef _base_hpp_include_
#define _base_hpp_include_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <atomic>

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

using uintptr = uintptr_t;

using f32 = float;
using f64 = double;

constexpr isize cache_line_size = 64;

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
static_assert(sizeof(isize) == sizeof(isize), "Mismatched (i/u)size");
static_assert(sizeof(void(*)(void)) == sizeof(void*), "Function pointers and data pointers must be of the same width");
static_assert(sizeof(void(*)(void)) == sizeof(uintptr), "Mismatched pointer types");

namespace meta {
template<typename A, typename B>
struct SameType { static constexpr bool value = false; };

template<typename A>
struct SameType<A, A> { static constexpr bool value = true; };

template<typename A, typename B>
constexpr bool same_type = SameType<A, B>::value;
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

template<typename Value, typename Error>
struct Result {
	static_assert(!meta::same_type<Value, Error>, "Value and Error must not be the same types");
	union {
		Value value;
		Error error{};
	};
	u8 has_value;

	Value unwrap(){
		ensure(has_value, "Cannot unwrap error result");
		return value;
	}

	Error unwrap_error(){
		ensure(has_value, "Cannot unwrap_error value result");
		return error;
	}

	Value or_else(Value alt){
		if(has_value){
			return value;
		} else {
			return alt;
		}
	}

	Result() : has_value{false} {}
	Result(Value v) : value{v}, has_value{true} {}
	Result(Error v) : value{v}, has_value{false} {}

	bool ok(){ return has_value; }
};

template<typename T>
struct Slice {
	T*   _data   = nullptr;
	isize _length = 0;

	T& operator[](isize idx) noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	T const& operator[](isize idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	Slice<T> operator[](Pair<isize> range){
		isize from = range.a;
		isize to = range.b;

		bounds_check_assert(from >= 0 && from < _length && to >= 0 && to <= _length && from <= to, "Index to sub-slice is out of bounds");

		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}

	// Get the sub-slice of interval a..b (end exclusive)
	Slice<T> slice(isize from, isize to){
		bounds_check_assert(
				from >= 0 && from < _length &&
				to >= 0 && to <= _length &&
				from <= to,
				"Index to sub-slice is out of bounds");
		Slice<T> res;
		res._length = to - from;
		res._data = &_data[from];
		return res;
	}

	// Get the sub-slice of elements after index (inclusive)
	Slice<T> slice_right(isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		auto res = Slice<T>(&_data[idx], _length - idx);
		return res;
	}

	// Get the sub-slice of elements before index (exclusive)
	Slice<T> slice_left(isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		auto res = Slice(_data, idx);
		return res;
	}

	Slice(){}
	explicit Slice(T* data, isize len) : _data{data}, _length{len} {}

	// Accessors
	isize len() const { return _length; }
	T* raw_data() const { return _data; }
};

constexpr isize mem_KiB = 1024ll;
constexpr isize mem_MiB = 1024ll * 1024ll;
constexpr isize mem_GiB = 1024ll * 1024ll * 1024ll;

void mem_set(void* p, byte val, isize nbytes);

void mem_copy(void* dest, void const * src, isize nbytes);

void mem_copy_no_overlap(void* dest, void const * src, isize nbytes);

i32 mem_compare(void const * a, void const * b, isize nbytes);

static inline
bool mem_valid_alignment(isize a){
	return ((a & (a - 1)) == 0) && (a > 0);
}

static inline
uintptr mem_align_forward_ptr(uintptr p, uintptr a){
	debug_assert(mem_valid_alignment(a), "Invalid memory alignment");
	uintptr mod = p & (a - 1); // Fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

static inline
isize mem_align_forward_size(isize p, isize a){
	debug_assert(mem_valid_alignment(a), "Invalid size alignment");
	isize mod = p & (a - 1); // Fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

//// UTF-8 ////////////////////////////////////////////////////////////////////
struct Utf8EncodeResult {
	byte bytes[4];
	i32 len;
};

struct Utf8DecodeResult {
	rune codepoint;
	i32 len;
};

constexpr rune ERROR = 0xfffd;

constexpr Utf8EncodeResult ERROR_ENCODED = {
	.bytes = {0xef, 0xbf, 0xbd},
	.len = 0,
};

Utf8EncodeResult utf8_encode(rune c);

Utf8DecodeResult utf8_decode(Slice<byte> buf);

struct Utf8Iterator {
	Slice<byte> data;
	isize current;
};

bool iter_next(Utf8Iterator* it, rune* r, i32* len);

bool iter_prev(Utf8Iterator* it, rune* r, i32* len);

rune iter_next(Utf8Iterator* it);

rune iter_prev(Utf8Iterator* it);

//// String ///////////////////////////////////////////////////////////////////
static inline
isize cstring_len(char const* cstr){
	isize size = 0;
	for(isize i = 0; cstr[i] != 0; i += 1){
		size += 1;
	}
	return size;
}

struct String {
private:
	byte const * _data = nullptr;
	isize _length = 0;
public:

	// Implict conversion, this is one of the very vew places an implicit
	// conversion is made in the library, mostly to write C-strings more
	// ergonomically.
	String(){}
	String(const char* cs) : _data{(byte const*)cs}, _length{cstring_len(cs)}{}
	explicit String(byte const* p, isize n) : _data{p}, _length{n}{}

	byte operator[](isize idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx <_length, "Out of bounds index on string");
		return _data[idx];
	}

	String operator[](Pair<isize> range) const noexcept {
		isize from = range.a;
		isize to = range.b;
		bounds_check_assert(from >= 0 && from < _length && to >= 0 && to <= _length && from <= to, "Index to sub-string is out of bounds");

		return String(&_data[from], to - from);
	}

	String slice_right(isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		auto res = String(&_data[idx], _length - idx);
		return res;
	}

	String slice_left(isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		auto res = String(_data, idx);
		return res;
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
	isize len() const { return _length; }
	byte* raw_data() const { return (byte*)_data; }
};

static inline
String string_from_cstring(char const* data){
	return String((byte const*)data, cstring_len(data));
}

static inline
String string_from_cstring(char const* data, isize start, isize length){
	return String((byte const*) &data[start], length);
}

static inline
String string_from_bytes(Slice<byte> buf){
	return String(buf.raw_data(), buf.len());
}

Utf8Iterator str_iterator(String s);

Utf8Iterator str_iterator_reversed(String s);

//// Memory ///////////////////////////////////////////////////////////////////
constexpr u32 mem_protection_none    = 0;
constexpr u32 mem_protection_read    = (1 << 0);
constexpr u32 mem_protection_write   = (1 << 1);
constexpr u32 mem_protection_execute = (1 << 2);

enum class AllocatorMode : u32 {
	Query    = 0, // Query allocator's capabilities
	Alloc    = 1, // Allocate a chunk of memory
	Resize   = 2, // Resize an allocation in-place
	Free     = 3, // Mark allocation as free
	FreeAll  = 4, // Mark allocations as free
	Realloc  = 5, // Re-allocate pointer
};

enum class AllocatorCapability : u32 {
	AllocAny = 1 << 0, // Can alloc any size
	FreeAny  = 1 << 1, // Can free in any order
	FreeAll  = 1 << 2, // Can free all allocations
	Resize   = 1 << 3, // Can resize in-place
	AlignAny = 1 << 4, // Can alloc aligned to any alignment
};

// Memory allocator method
using AllocatorFunc = void* (*) (
	void* impl,
	AllocatorMode op,
	void* old_ptr,
	isize old_size,
	isize size,
	isize align,
	u32* capabilities
);

// Memory allocator interface
struct Allocator {
	void* data = 0;
	AllocatorFunc func = 0;

	void* alloc(isize nbytes, isize align);

	void* resize(void* ptr, isize new_size);

	void free(void* ptr, isize old_size, isize align);

	void* realloc(void* ptr, isize old_size, isize new_size, isize align);

	void free_all();
};


template<typename T> [[nodiscard]]
T* make(Allocator a){
	T* p = (T*)a.alloc(sizeof(T), alignof(T));
	return p;
}

template<typename T> [[nodiscard]]
Slice<T> make(Allocator a, isize elems){
	T* p = (T*)a.alloc(sizeof(T) * elems, alignof(T));
	return Slice<T>(p, p == nullptr ? 0 : elems);
}

template<typename T>
void destroy(Allocator a, T* obj){
	a.free(obj, sizeof(T));
}

template<typename T>
void destroy(Allocator a, Slice<T> s){
	a.free(s.raw_data(), sizeof(T) * s.len());
}

template<typename T>
void destroy(Allocator a, String s){
	a.free(s.raw_data(), sizeof(T) * s.len(), alignof(T));
}

//// Virtual Memory ///////////////////////////////////////////////////////////
constexpr isize mem_page_size = 4096;

struct PageBlock {
	isize reserved;
	isize commited;
	void* pointer;

	void* push(isize nbytes);

	void pop(isize nbytes);

	void destroy();

	static PageBlock make(isize nbytes);
};

void* virtual_reserve(isize nbytes);

void virtual_release(void* pointer, isize nbytes);

bool virtual_protect(void* pointer, u32 prot);

void* virtual_commit(void* pointer, isize nbytes);

void virtual_decommit(void* pointer, isize nbytes);


//// Arena ////////////////////////////////////////////////////////////////////
enum struct ArenaType : u32 {
	Buffer  = 0,
	Virtual = 1,
};

struct Arena {
	PageBlock data;
	isize offset;
	uintptr last_allocation;
	ArenaType type;

	void* alloc(isize nbytes, isize align);

	void* resize_in_place(void* ptr, isize new_size);

	void* realloc(void* ptr, isize old_size, isize new_size, isize align);

	void free_all();

	void destroy();

	Allocator as_allocator();

	static Arena from_buffer(Slice<u8> buf);

	static Arena make_virtual(isize reserve);
};

//// Dynamic Array ////////////////////////////////////////////////////////////
template<typename T>
struct DynamicArray {
	T*        _data;
	isize     _capacity;
	isize     _length;
	Allocator _allocator;

	T& operator[](isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds access to dynamic array");
		return _data[idx];
	}

	T const& operator[](isize idx) const{
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds access to dynamic array");
		return _data[idx];
	}

	Slice<T> operator[](Pair<isize> range){
		isize from = range.a;
		isize to = range.b;

		bounds_check_assert(from >= 0 && from < _length && to >= 0 && to <= _length && from <= to, "Index to sub-slice is out of bounds");

		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}

	void clear(){
		_length = 0;
	}

	void destroy(){
		_allocator.free(_data, _capacity * sizeof(T), alignof(T));
	}

	Slice<T> as_slice(){
		return Slice<T>(_data, _length);
	}

	bool append(T elem){
		[[unlikely]] if(_length >= _capacity){
			isize new_cap = mem_align_forward_size(_length * 2, 16);
			auto new_data = (T*)_allocator.realloc(
					_data,
					_capacity * sizeof(T),
					new_cap, alignof(T));
			if(new_data == nullptr){
				return false;
			}
			_data = new_data;
		}
		_data[_length] = elem;
		_length += 1;
		return true;
	}

	bool pop(){
		if(_length <= 0){ return false; }
		_length -= 1;
		auto v = _data[_length];
		return v;
	}

	bool insert(isize idx, T elem){
		bounds_check_assert(idx >= 0 && idx <= _length, "Out of bounds index to insert_swap");
		if(idx == _length){ return append(this, elem); }

		bool ok = append(this, elem);
		if(!ok){ return false; }

		isize nbytes = sizeof(T) * (_length - 1 - idx);
		mem_copy(&_data[idx + 1], &_data[idx], nbytes);
		_data[idx] = elem;
		return true;
	}

	bool insert_swap(isize idx, T elem){
		bounds_check_assert(idx >= 0 && idx <= _length, "Out of bounds index to insert_swap");
		if(idx == _length){ return append(this, elem); }

		bool ok = append(this, _data[idx]);
		[[unlikely]] if(!ok){ return false; }
		_data[idx] = elem;

		return true;
	}

	void remove_swap(isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds index to remove_swap");
		T last = _data[_length - 1];
		_data[idx] = last;
		_length -= 1;
	}

	void remove(isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds index to remove");
		isize nbytes = sizeof(T) * (_length - idx + 1);
		mem_copy(&_data[idx], &_data[idx+1], nbytes);
		_length -= 1;
	}

	static DynamicArray<T> make(Allocator alloc, isize initial_cap = 16){
		DynamicArray<T> arr;
		arr._capacity = initial_cap;
		arr._length = 0;
		arr._allocator = alloc;
		arr._data = (T*)alloc.alloc(initial_cap * sizeof(T), alignof(T));
		return arr;
	}

	// Accessors
	isize len() const { return _length; }
	isize cap() const { return _capacity; }
	T* raw_data() const { return _data; }
	Allocator allocator() const { return _allocator; }
};

//// Map //////////////////////////////////////////////////////////////////////
#include "debug_print.cpp"

static inline
u64 map_hash_fnv64(byte const * data, isize nbytes){
	constexpr u64 prime = 0x100000001b3ull;
	constexpr u64 offset_basis = 0xcbf29ce484222325ull;

	u64 hash = offset_basis;
	for(isize i = 0; i < nbytes; i ++){
		auto b = u64(data[i]);
		hash = hash ^ b;
		hash = hash * prime;
	}

	return hash | u64(hash == 0);
}

template<typename K, typename V>
struct MapSlot {
	K key;
	V value;
	u64 hash;
	MapSlot* next;
};

template<typename K, typename V>
struct Map {
	MapSlot<K, V>* base_slots;
	isize capacity;
	Allocator allocator;
};

template<typename K, typename V>
Map<K, V> map_create(Allocator allocator, isize capacity){
	ensure((capacity & (capacity - 1)) == 0, "Capacity must be a power of 2");
	Map<K, V> m;
	auto base_slots = allocator.alloc(capacity * sizeof(MapSlot<K, V>), alignof(MapSlot<K, V>));
	if(!base_slots){ return m; }

	m.base_slots = static_cast<MapSlot<K, V>*>(base_slots);
	m.capacity = capacity;
	m.allocator = allocator;

	return m;
}

template<typename K, typename V>
void destroy(Map<K, V>* map){
	constexpr isize slot_size = sizeof(MapSlot<K, V>);
	constexpr isize slot_align = alignof(MapSlot<K, V>);
	for(isize i = 0; i < map->capacity; i++){
		MapSlot<K, V>* next = nullptr;
		for(MapSlot<K, V>* slot = map->base_slots[i].next; slot != nullptr; slot = next){
			next = slot->next;
			map->allocator.free((byte*)slot, slot_size, slot_align);
		}
	}

	mem_free(map->allocator, (byte*)map->base_slots, slot_size * map->capacity, slot_align);
}

template<typename K, typename V>
Pair<isize, u64> map_slot_offset(Map<K, V>* map, K key){
	auto data   = (byte const*)&key;
	auto hash   = map_hash_fnv64(data, sizeof(key));
	isize pos    = isize(hash & (map->capacity - 1));
	return {pos, hash};
}

template<typename V>
Pair<isize, u64> map_slot_offset(Map<String, V>* map, String key){
	auto hash   = map_hash_fnv64(key.raw_data(), key.len());
	isize pos    = isize(hash & (map->capacity - 1));
	return {pos, hash};
}

template<typename K, typename V, typename PK = K>
Pair<V, bool> map_get(Map<K, V>* map, PK key){
	auto map_key = K(key);
	auto [pos, hash] = map_slot_offset(map, map_key);

	for(auto slot = &map->base_slots[pos]; slot != nullptr; slot = slot->next){
		bool hit = (slot->hash == hash) && (slot->key == map_key);
		if(hit){
			return {slot->value, true};
		}
	}
	return {V{}, false};
}

template<typename K, typename V, typename PK = K, typename PV = V>
bool map_set(Map<K, V>* map, PK map_key, PV map_val){
	auto key = static_cast<K>(map_key);
	auto val = static_cast<V>(map_val);

	auto [pos, hash] = map_slot_offset(map, key);

	if(map->base_slots[pos].hash == 0){
		map->base_slots[pos].key   = key;
		map->base_slots[pos].value = val;
		map->base_slots[pos].hash  = hash;
	}
	else {
		MapSlot<K, V>* new_slot = make<MapSlot<K, V>>(map->allocator);
		if(new_slot == nullptr){
			return false;
		}

		*new_slot = map->base_slots[pos];

		map->base_slots[pos].key   = key;
		map->base_slots[pos].value = val;
		map->base_slots[pos].hash  = hash;
		map->base_slots[pos].next  = new_slot;
	}
	return true;
}

//// Heap Allocator (LibC) ////////////////////////////////////////////////////
Allocator heap_allocator();

//// String Utilities /////////////////////////////////////////////////////////
String str_trim(String s, String cutset);

String str_trim_leading(String s, String cutset);

String str_trim_trailing(String s, String cutset);

isize str_rune_count(String s);

bool str_starts_with(String s, String prefix);

bool str_ends_with(String s, String suffix);

isize str_find(String s, String substr, isize start = 0);

[[nodiscard]]
String str_clone(String s, Allocator allocator);

[[nodiscard]]
String str_concat(String s0, String s1, Allocator allocator);

//// String Builder ///////////////////////////////////////////////////////////
// struct StringBuilder {
// 	DynamicArray<byte> buffer;
// };
//
// StringBuilder str_builder_create(Allocator allocator);
//
// void str_append(StringBuilder* sb, String v);
// void str_append(StringBuilder* sb, rune v);
// void str_append(StringBuilder* sb, i64 v);
// void str_append(StringBuilder* sb, f64 v);
// void str_append(StringBuilder* sb, bool v);

//// SIMD /////////////////////////////////////////////////////////////////////
namespace simd {
#define VECTOR_DECL(T, N) __attribute__((vector_size((N) * sizeof(T)))) T;

// 128-bit
using i8x16 = VECTOR_DECL(i8, 16);
using i16x8 = VECTOR_DECL(i16, 8);
using i32x4 = VECTOR_DECL(i32, 4);
using i64x2 = VECTOR_DECL(i64, 2);

using u8x16 = VECTOR_DECL(u8, 16);
using u16x8 = VECTOR_DECL(u16, 8);
using u32x4 = VECTOR_DECL(u32, 4);
using u64x2 = VECTOR_DECL(u64, 2);

using f32x4 = VECTOR_DECL(f32, 4);
using f64x2 = VECTOR_DECL(f64, 2);

// 256-bit
using i8x32  = VECTOR_DECL(i8, 32);
using i16x16 = VECTOR_DECL(i16, 16);
using i32x8  = VECTOR_DECL(i32, 8);
using i64x4  = VECTOR_DECL(i64, 4);

using u8x32  = VECTOR_DECL(u8, 32);
using u16x16 = VECTOR_DECL(u16, 16);
using u32x8  = VECTOR_DECL(u32, 8);
using u64x4  = VECTOR_DECL(u64, 4);

using f32x8 = VECTOR_DECL(f32, 8);
using f64x4 = VECTOR_DECL(f64, 4);
}

#endif /* Include guard */

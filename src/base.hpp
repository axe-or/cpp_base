#ifndef _base_hpp_include_
#define _base_hpp_include_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
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
struct Option {
	T _value;
	bool _has_value = false;

	T unwrap(){
		if(_has_value){
			return _value;
		}
		panic("Attempt to unwrap a null option");
	}

	T unwrap_unchecked(){
		return _value;
	}

	bool ok() const { return _has_value; }

	template<typename U>
	T or_else(U alt){
		if(_has_value){
			return _value;
		}
		return static_cast<T>(alt);
	}

	void clear(){
		_has_value = false;
	}

	Option() : _has_value{false}{}

	// Implicit conversion
	Option(T val) : _value{val}, _has_value{true} {}
};

template<typename T>
struct Option<T*> {
	T* _value;

	T* unwrap(){
		if(_value != nullptr){
			return _value;
		}
		panic("Attempt to unwrap a null option");
	}

	T* unwrap_unchecked(){
		return _value;
	}

	bool ok() const { return _value != nullptr; }

	T* or_else(T* alt){
		if(_value != nullptr){
			return _value;
		}
		return alt;
	}

	void clear(){
		_has_value = false;
	}

	Option() : _value{nullptr} {}

	Option(T* p) : _value{p} {}
};

template<typename T>
struct Slice {
	T*   _data   = nullptr;
	Size _length = 0;

	T& operator[](Size idx) noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	T const& operator[](Size idx) const noexcept {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	Slice<T> operator[](Pair<Size> range){
		Size from = range.a;
		Size to = range.b;

		bounds_check_assert(from >= 0 && from < _length && to >= 0 && to <= _length && from <= to, "Index to sub-slice is out of bounds");

		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}

	// Get the sub-slice of interval a..b (end exclusive)
	Slice<T> slice(Size from, Size to){
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
	Slice<T> slice_right(Size idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		auto res = Slice<T>(&_data[idx], _length - idx);
		return res;
	}

	// Get the sub-slice of elements before index (exclusive)
	Slice<T> slice_left(Size idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		auto res = Slice(_data, idx);
		return res;
	}

	Slice(){}
	explicit Slice(T* data, Size len) : _data{data}, _length{len} {}

	// Accessors
	Size len() const { return _length; }
	T* raw_data() const { return _data; }
};

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

	String slice_right(Size idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to sub-slice is out of bounds");
		auto res = String(&_data[idx], _length - idx);
		return res;
	}

	String slice_left(Size idx){
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
	Size len() const { return _length; }
	Byte* raw_data() const { return (Byte*)_data; }
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
	return String(buf.raw_data(), buf.len());
}

Utf8Iterator str_iterator(String s);

Utf8Iterator str_iterator_reversed(String s);

//// Memory ///////////////////////////////////////////////////////////////////
constexpr U32 mem_protection_none    = 0;
constexpr U32 mem_protection_read    = (1 << 0);
constexpr U32 mem_protection_write   = (1 << 1);
constexpr U32 mem_protection_execute = (1 << 2);

enum class AllocatorMode : U32 {
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
};

// Memory allocator method
using AllocatorFunc = void* (*) (
	void* impl,
	AllocatorMode op,
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

void mem_free(Allocator a, void* ptr, Size old_size, Size align);

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
	return Slice<T>(p, p == nullptr ? 0 : elems);
}

template<typename T>
void destroy(Allocator a, T* obj){
	mem_free(a, obj, sizeof(T));
}

template<typename T>
void destroy(Allocator a, Slice<T> s){
	mem_free(a, s.raw_data(), sizeof(T) * s.len());
}

template<typename T>
void destroy(Allocator a, String s){
	mem_free(a, s.raw_data(), sizeof(T) * s.len(), alignof(T));
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
	T*        _data;
	Size      _capacity;
	Size      _length;
	Allocator _allocator;

	T& operator[](Size idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds access to dynamic array");
		return _data[idx];
	}

	T const& operator[](Size idx) const{
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds access to dynamic array");
		return _data[idx];
	}

	Slice<T> operator[](Pair<Size> range){
		Size from = range.a;
		Size to = range.b;

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
		mem_free(_allocator, _data, _capacity * sizeof(T), alignof(T));
	}

	Slice<T> as_slice(){
		return Slice<T>(_data, _length);
	}

	bool append(T elem){
		[[unlikely]] if(_length >= _capacity){
			Size new_cap = mem_align_forward_size(_length * 2, 16);
			auto new_data = (T*)mem_realloc(
					_allocator,
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

	Option<T> pop(){
		if(_length <= 0){ return {}; }
		_length -= 1;
		auto v = _data[_length];
		return v;
	}

	bool insert(Size idx, T elem){
		bounds_check_assert(idx >= 0 && idx <= _length, "Out of bounds index to insert_swap");
		if(idx == _length){ return append(this, elem); }

		bool ok = append(this, elem);
		if(!ok){ return false; }

		Size nbytes = sizeof(T) * (_length - 1 - idx);
		mem_copy(&_data[idx + 1], &_data[idx], nbytes);
		_data[idx] = elem;
		return true;
	}

	bool insert_swap(Size idx, T elem){
		bounds_check_assert(idx >= 0 && idx <= _length, "Out of bounds index to insert_swap");
		if(idx == _length){ return append(this, elem); }

		bool ok = append(this, _data[idx]);
		[[unlikely]] if(!ok){ return false; }
		_data[idx] = elem;

		return true;
	}

	void remove_swap(Size idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds index to remove_swap");
		T last = _data[_length - 1];
		_data[idx] = last;
		_length -= 1;
	}

	void remove(Size idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds index to remove");
		Size nbytes = sizeof(T) * (_length - idx + 1);
		mem_copy(&_data[idx], &_data[idx+1], nbytes);
		_length -= 1;
	}

	static DynamicArray<T> make(Allocator alloc, Size initial_cap = 16){
		DynamicArray<T> arr;
		arr._capacity = initial_cap;
		arr._length = 0;
		arr._allocator = alloc;
		arr._data = (T*)mem_alloc(alloc, initial_cap * sizeof(T), alignof(T));
		return arr;
	}


	// Accessors
	Size len() const { return _length; }
	Size cap() const { return _capacity; }
	T* raw_data() const { return _data; }
	Allocator allocator() const { return _allocator; }
};



//// Map //////////////////////////////////////////////////////////////////////
#include "debug_print.cpp"

static inline
U64 map_hash_fnv64(Byte const * data, Size nbytes){
	constexpr U64 prime = 0x100000001b3ull;
	constexpr U64 offset_basis = 0xcbf29ce484222325ull;

	U64 hash = offset_basis;
	for(Size i = 0; i < nbytes; i ++){
		auto b = U64(data[i]);
		hash = hash ^ b;
		hash = hash * prime;
	}

	return hash | U64(hash == 0);
}

template<typename K, typename V>
struct MapSlot {
	K key;
	V value;
	U64 hash;
	MapSlot* next;
};

template<typename K, typename V>
struct Map {
	MapSlot<K, V>* base_slots;
	Size capacity;
	Allocator allocator;
};

template<typename K, typename V>
Map<K, V> map_create(Allocator allocator, Size capacity){
	ensure((capacity & (capacity - 1)) == 0, "Capacity must be a power of 2");
	Map<K, V> m;
	auto base_slots = mem_alloc(allocator, capacity * sizeof(MapSlot<K, V>), alignof(MapSlot<K, V>));
	if(!base_slots){ return m; }

	m.base_slots = static_cast<MapSlot<K, V>*>(base_slots);
	m.capacity = capacity;
	m.allocator = allocator;

	return m;
}

template<typename K, typename V>
void destroy(Map<K, V>* map){
	constexpr Size slot_size = sizeof(MapSlot<K, V>);
	constexpr Size slot_align = alignof(MapSlot<K, V>);
	for(Size i = 0; i < map->capacity; i++){
		MapSlot<K, V>* next = nullptr;
		for(MapSlot<K, V>* slot = map->base_slots[i].next; slot != nullptr; slot = next){
			next = slot->next;
			mem_free(map->allocator, (Byte*)slot, slot_size, slot_align);
		}
	}

	mem_free(map->allocator, (Byte*)map->base_slots, slot_size * map->capacity, slot_align);
}

template<typename K, typename V>
Pair<Size, U64> map_slot_offset(Map<K, V>* map, K key){
	auto data   = (Byte const*)&key;
	auto hash   = map_hash_fnv64(data, sizeof(key));
	Size pos    = Size(hash & (map->capacity - 1));
	return {pos, hash};
}

template<typename V>
Pair<Size, U64> map_slot_offset(Map<String, V>* map, String key){
	auto hash   = map_hash_fnv64(raw_data(key), len(key));
	Size pos    = Size(hash & (map->capacity - 1));
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

Size str_rune_count(String s);

bool str_starts_with(String s, String prefix);

bool str_ends_with(String s, String suffix);

Size str_find(String s, String substr, Size start = 0);

[[nodiscard]]
String str_clone(String s, Allocator allocator);

[[nodiscard]]
String str_concat(String s0, String s1, Allocator allocator);

//// String Builder ///////////////////////////////////////////////////////////
// struct StringBuilder {
// 	DynamicArray<Byte> buffer;
// };
//
// StringBuilder str_builder_create(Allocator allocator);
//
// void str_append(StringBuilder* sb, String v);
// void str_append(StringBuilder* sb, Rune v);
// void str_append(StringBuilder* sb, I64 v);
// void str_append(StringBuilder* sb, F64 v);
// void str_append(StringBuilder* sb, bool v);

//// SIMD /////////////////////////////////////////////////////////////////////
namespace simd {
#define VECTOR_DECL(T, N) __attribute__((vector_size((N) * sizeof(T)))) T;

// 128-bit
using I8x16 = VECTOR_DECL(I8, 16);
using I16x8 = VECTOR_DECL(I16, 8);
using I32x4 = VECTOR_DECL(I32, 4);
using I64x2 = VECTOR_DECL(I64, 2);

using U8x16 = VECTOR_DECL(U8, 16);
using U16x8 = VECTOR_DECL(U16, 8);
using U32x4 = VECTOR_DECL(U32, 4);
using U64x2 = VECTOR_DECL(U64, 2);

using F32x4 = VECTOR_DECL(F32, 4);
using F64x2 = VECTOR_DECL(F64, 2);

// 256-bit
using I8x32  = VECTOR_DECL(I8, 32);
using I16x16 = VECTOR_DECL(I16, 16);
using I32x8  = VECTOR_DECL(I32, 8);
using I64x4  = VECTOR_DECL(I64, 4);

using U8x32  = VECTOR_DECL(U8, 32);
using U16x16 = VECTOR_DECL(U16, 16);
using U32x8  = VECTOR_DECL(U32, 8);
using U64x4  = VECTOR_DECL(U64, 4);

using F32x8 = VECTOR_DECL(F32, 8);
using F64x4 = VECTOR_DECL(F64, 4);
}

#endif /* Include guard */

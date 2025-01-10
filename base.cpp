#include "base.hpp"
#include <stdio.h>

//// Assert ////////////////////////////////////////////////////////////////////
void debug_assert_ex(bool pred, cstring msg, Source_Location loc){
	#if defined(NDEBUG) || defined(RELEASE_MODE)
		(void)pred; (void)msg;
	#else
	[[unlikely]]
	if(!pred){
		fprintf(stderr, "(%s:%d) Failed assert: %s\n", loc.filename, loc.line, msg);
		abort();
	}
	#endif
}

void panic_assert_ex(bool pred, cstring msg, Source_Location loc){
	[[unlikely]]
	if(!pred){
		fprintf(stderr, "[%s:%d %s()] Failed assert: %s\n", loc.filename, loc.line, loc.caller_name, msg);
		abort();
	}
}

void bounds_check_assert_ex(bool pred, cstring msg, Source_Location loc){
	#if defined(DISABLE_BOUNDS_CHECK)
		(void)pred; (void)msg;
	#else
	[[unlikely]]
	if(!pred){
		fprintf(stderr, "(%s:%d) Failed bounds check: %s\n", loc.filename, loc.line, msg);
		abort();
	}
	#endif
}

[[noreturn]]
void panic(char* const msg){
	fprintf(stderr, "Panic: %s\n", msg);
	abort();
}

[[noreturn]]
void unimplemented(){
	fprintf(stderr, "Unimplemented code\n");
	abort();
}

//// Memory ////////////////////////////////////////////////////////////////////
namespace mem {
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

static inline
bool valid_alignment(isize align){
	return (align & (align - 1)) == 0 && (align != 0);
}

void set(void* p, byte val, isize nbytes){
	mem_set_impl(p, val, nbytes);
}

void copy(void* dest, void const * src, isize nbytes){
	mem_copy_impl(dest, src, nbytes);
}

void copy_no_overlap(void* dest, void const * src, isize nbytes){
	mem_copy_no_overlap_impl(dest, src, nbytes);
}

i32 compare(void const * a, void const * b, isize nbytes){
	return mem_compare_impl(a, b, nbytes);
}

uintptr align_forward_ptr(uintptr p, uintptr a){
	debug_assert(valid_alignment(a), "Invalid memory alignment");
	uintptr mod = p & (a - 1);
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

uintptr align_forward_size(isize p, isize a){
	debug_assert(a > 0, "Invalid size alignment");
	isize mod = p % a;
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}
} /* Namespace mem */

//// UTF-8 /////////////////////////////////////////////////////////////////////
namespace utf8 {
constexpr i32 RANGE1 = 0x7f;
constexpr i32 RANGE2 = 0x7ff;
constexpr i32 RANGE3 = 0xffff;
constexpr i32 RANGE4 = 0x10ffff;

constexpr i32 UTF16_SURROGATE1 = 0xd800;
constexpr i32 UTF16_SURROGATE2 = 0xdfff;

constexpr i32 MASK2 = 0x1f; /* 0001_1111 */
constexpr i32 MASK3 = 0x0f; /* 0000_1111 */
constexpr i32 MASK4 = 0x07; /* 0000_0111 */

constexpr i32 MASKX = 0x3f; /* 0011_1111 */

constexpr i32 SIZE2 = 0xc0; /* 110x_xxxx */
constexpr i32 SIZE3 = 0xe0; /* 1110_xxxx */
constexpr i32 SIZE4 = 0xf0; /* 1111_0xxx */

constexpr i32 CONT = 0x80; /* 10xx_xxxx */

static inline
bool is_continuation_byte(rune c){
	static const rune CONTINUATION1 = 0x80;
	static const rune CONTINUATION2 = 0xbf;
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}

Encode_Result encode(rune c){
	Encode_Result res = {};

	if(is_continuation_byte(c) ||
	   (c >= UTF16_SURROGATE1 && c <= UTF16_SURROGATE2) ||
	   (c > RANGE4))
	{
		return ERROR_ENCODED;
	}

	if(c <= RANGE1){
		res.len = 1;
		res.bytes[0] = c;
	}
	else if(c <= RANGE2){
		res.len = 2;
		res.bytes[0] = SIZE2 | ((c >> 6) & MASK2);
		res.bytes[1] = CONT  | ((c >> 0) & MASKX);
	}
	else if(c <= RANGE3){
		res.len = 3;
		res.bytes[0] = SIZE3 | ((c >> 12) & MASK3);
		res.bytes[1] = CONT  | ((c >> 6) & MASKX);
		res.bytes[2] = CONT  | ((c >> 0) & MASKX);
	}
	else if(c <= RANGE4){
		res.len = 4;
		res.bytes[0] = SIZE4 | ((c >> 18) & MASK4);
		res.bytes[1] = CONT  | ((c >> 12) & MASKX);
		res.bytes[2] = CONT  | ((c >> 6)  & MASKX);
		res.bytes[3] = CONT  | ((c >> 0)  & MASKX);
	}
	return res;
}

constexpr Decode_Result DECODE_ERROR = { .codepoint = ERROR, .len = 0 };

Decode_Result decode(Slice<byte> s){
	Decode_Result res = {};
	byte* buf = s.raw_data();
	isize len = s.size();

	if(s.empty()){ return DECODE_ERROR; }

	u8 first = buf[0];

	if((first & CONT) == 0){
		res.len = 1;
		res.codepoint |= first;
	}
	else if ((first & ~MASK2) == SIZE2 && len >= 2){
		res.len = 2;
		res.codepoint |= (buf[0] & MASK2) << 6;
		res.codepoint |= (buf[1] & MASKX) << 0;
	}
	else if ((first & ~MASK3) == SIZE3 && len >= 3){
		res.len = 3;
		res.codepoint |= (buf[0] & MASK3) << 12;
		res.codepoint |= (buf[1] & MASKX) << 6;
		res.codepoint |= (buf[2] & MASKX) << 0;
	}
	else if ((first & ~MASK4) == SIZE4 && len >= 4){
		res.len = 4;
		res.codepoint |= (buf[0] & MASK4) << 18;
		res.codepoint |= (buf[1] & MASKX) << 12;
		res.codepoint |= (buf[2] & MASKX) << 6;
		res.codepoint |= (buf[3] & MASKX) << 0;
	}
	else {
		return DECODE_ERROR;
	}

	// Validation step
	if(res.codepoint >= UTF16_SURROGATE1 && res.codepoint <= UTF16_SURROGATE2){
		return DECODE_ERROR;
	}
	if(res.len > 1 && !is_continuation_byte(buf[1])){
		return DECODE_ERROR;
	}
	if(res.len > 2 && !is_continuation_byte(buf[2])){
		return DECODE_ERROR;
	}
	if(res.len > 3 && !is_continuation_byte(buf[3])){
		return DECODE_ERROR;
	}

	return res;
}

bool Iterator::next(rune* r, i8* len){
	if(this->current >= this->data.size()){ return 0; }

	Decode_Result res = decode(this->data.sub(current));
	*r = res.codepoint;
	*len = res.len;

	if(res.codepoint == DECODE_ERROR.codepoint){
		*len = 1;
	}

	this->current += res.len;

	return 1;
}

rune Iterator::next(){
	i8 n = 0;
	rune r;
	if(!next(&r, &n)) return 0;
	return r;
}

rune Iterator::prev(){
	i8 n = 0;
	rune r;
	if(!prev(&r, &n)) return 0;
	return r;
}


// Steps iterator backward and puts rune and its length into pointers,
// returns false when finished.
bool Iterator::prev(rune* r, i8* len){
	if(this->current <= 0){ return false; }

	this->current -= 1;
	while(is_continuation_byte(this->data[this->current])){
		this->current -= 1;
	}

	Decode_Result res = decode(this->data.sub(current));
	*r = res.codepoint;
	*len = res.len;
	return true;
}
} /* Namespace utf8 */

//// Strings ///////////////////////////////////////////////////////////////////
isize String::rune_count(){
	[[maybe_unused]] rune r;
	[[maybe_unused]] i8 n;
	auto it = iterator();
	isize size = 0;
	while(it.next(&r, &n)) { size += 1; }
	return size;
}

String String::sub(isize from, isize to){
	bounds_check_assert(from <= to, "Improper slicing range");
	bounds_check_assert(from >= 0 && from < _length, "Index to sub-string is out of bounds");
	bounds_check_assert(to >= 0 && to <= _length, "Index to sub-string is out of bounds");
	String s;
	s._length = to - from;
	s._data = &_data[from];
	return s;
}

constexpr isize MAX_CUTSET_LEN = 128;

String String::trim(String cutset){
	String trimmed = this->trim_trailing(cutset).trim_trailing(cutset);
	return trimmed;
}

utf8::Iterator String::iterator(){
	return {
		.data = Slice<byte>::from_pointer((byte*)_data, _length),
		.current = 0,
	};
}

utf8::Iterator String::iterator_reversed(){
	return {
		.data = Slice<byte>::from_pointer((byte*)_data, _length),
		.current = _length,
	};
}

String String::trim_leading(String cutset){
	debug_assert(cutset.size() <= MAX_CUTSET_LEN, "Cutset string exceeds MAX_CUTSET_LEN");

	rune set[MAX_CUTSET_LEN] = {0};
	isize set_len = 0;
	isize cut_after = 0;

	decode_cutset: {
		rune c; i8 n;
		auto iter = cutset.iterator();

		isize i = 0;
		while(iter.next(&c, &n) && i < MAX_CUTSET_LEN){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	strip_cutset: {
		rune c; i8 n;
		auto iter = this->iterator();

		while(iter.next(&c, &n)){
			bool to_be_cut = false;
			for(isize i = 0; i < set_len; i += 1){
				if(set[i] == c){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_after += n;
			}
			else {
				break; // Reached first rune that isn't in cutset
			}

		}
	}

	unimplemented();
}

String String::trim_trailing(String cutset){
	debug_assert(cutset.size() <= MAX_CUTSET_LEN, "Cutset string exceeds MAX_CUTSET_LEN");

	rune set[MAX_CUTSET_LEN] = {0};
	isize set_len = 0;
	isize cut_until = _length;

	decode_cutset: {
		rune c; i8 n;
		auto iter = cutset.iterator();

		isize i = 0;
		while(iter.next(&c, &n) && i < MAX_CUTSET_LEN){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	strip_cutset: {
		rune c; i8 n;
		auto iter = this->iterator_reversed();

		while(iter.prev(&c, &n)){
			bool to_be_cut = false;
			for(isize i = 0; i < set_len; i += 1){
				if(set[i] == c){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_until -= n;
			}
			else {
				break; // Reached first rune that isn't in cutset
			}

		}
	}

	unimplemented();
}


//// Arena Allocator ///////////////////////////////////////////////////////////
namespace mem {
static
uintptr arena_required_mem(uintptr cur, isize nbytes, isize align){
	debug_assert(valid_alignment(align), "Alignment must be a power of 2");
	uintptr_t aligned  = align_forward_ptr(cur, align);
	uintptr_t padding  = (uintptr)(aligned - cur);
	uintptr_t required = padding + nbytes;
	return required;
}

void* Arena::alloc(isize size, isize align){
	uintptr base = (uintptr)data;
	uintptr current = (uintptr)base + (uintptr)offset;

	uintptr available = (uintptr)capacity - (current - base);
	uintptr required = arena_required_mem(current, size, align);

	if(required > available){
		return null;
	}

	offset += required;
	void* allocation = &data[offset - size];
	last_allocation = (uintptr)allocation;
	return allocation;
}

void Arena::free_all(){
	offset = 0;
}

void* Arena::resize(void* ptr, isize new_size){
	if((uintptr)ptr == last_allocation){
		uintptr base = (uintptr)data;
		uintptr current = base + (uintptr)offset;
		uintptr limit = base + (uintptr)capacity;
		isize last_allocation_size = current - last_allocation;

		if((current - last_allocation_size + new_size) > limit){
			return null; /* No space left*/
		}

		offset += new_size - last_allocation_size;
		return ptr;
	}

	return null;
}

static
void* arena_allocator_func(
	void* impl,
	Allocator_Op op,
	void* old_ptr,
	isize size,
	isize align,
	u32* capabilities)
{
	auto arena = (Arena*)impl;
	(void)old_ptr;
	using M = Allocator_Op;
	using C = Allocator_Capability;

	switch(op){
		case M::Alloc: {
			return arena->alloc(size, align);
		} break;

		case M::Free_All: {
			arena->free_all();
		} break;

		case M::Resize: {
			return arena->resize(old_ptr, size);
		} break;

		case M::Free: {} break;

		case M::Query: {
			*capabilities = u32(C::Align_Any) | u32(C::Free_All) | u32(C::Alloc_Any) | u32(C::Resize);
		} break;
	}

	return null;
}

Allocator Arena::allocator(){
	Allocator allocator = {
		._impl = this,
		._func = arena_allocator_func,
	};
	return allocator;
}

void arena_init(Arena* a, byte* data, isize len){
	a->capacity = len;
	a->data = data;
	a->offset = 0;
}

} /* Namespace mem */

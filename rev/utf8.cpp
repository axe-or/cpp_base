#include "base.hpp"

namespace utf8 {
constexpr I32 RANGE1 = 0x7f;
constexpr I32 RANGE2 = 0x7ff;
constexpr I32 RANGE3 = 0xffff;
constexpr I32 RANGE4 = 0x10ffff;

constexpr I32 UTF16_SURROGATE1 = 0xd800;
constexpr I32 UTF16_SURROGATE2 = 0xdfff;

constexpr I32 MASK2 = 0x1f; /* 0001_1111 */
constexpr I32 MASK3 = 0x0f; /* 0000_1111 */
constexpr I32 MASK4 = 0x07; /* 0000_0111 */

constexpr I32 MASKX = 0x3f; /* 0011_1111 */

constexpr I32 SIZE2 = 0xc0; /* 110x_xxxx */
constexpr I32 SIZE3 = 0xe0; /* 1110_xxxx */
constexpr I32 SIZE4 = 0xf0; /* 1111_0xxx */

constexpr I32 CONT = 0x80; /* 10xx_xxxx */

static inline
bool is_continuation_byte(Rune c){
	static const Rune CONTINUATION1 = 0x80;
	static const Rune CONTINUATION2 = 0xbf;
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}

EncodeResult encode(Rune c){
	EncodeResult res = {};

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

constexpr DecodeResult DECODE_ERROR = { .codepoint = ERROR, .len = 0 };

DecodeResult decode(Slice<Byte> s){
	DecodeResult res = {};
	Byte* buf = s.raw_data();
	Size len = s.len();

	if(s.empty()){ return DECODE_ERROR; }

	U8 first = buf[0];

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

bool Iterator::next(Rune* r, I32* len){
	if(this->current >= this->data.len()){ return 0; }

	DecodeResult res = decode(this->data.slice_right(current));
	*r = res.codepoint;
	*len = res.len;

	if(res.codepoint == DECODE_ERROR.codepoint){
		*len = 1;
	}

	this->current += res.len;

	return 1;
}

Rune Iterator::next(){
	I32 n = 0;
	Rune r;
	if(!next(&r, &n)) return 0;
	return r;
}

Rune Iterator::prev(){
	I32 n = 0;
	Rune r;
	if(!prev(&r, &n)) return 0;
	return r;
}

bool Iterator::prev(Rune* r, I32* len){
	if(this->current <= 0){ return false; }

	this->current -= 1;
	while(is_continuation_byte(this->data[this->current])){
		this->current -= 1;
	}

	DecodeResult res = decode(this->data.slice_right(current));
	*r = res.codepoint;
	*len = res.len;
	return true;
}
} /* Namespace utf8 */

#include "base.hpp"

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

EncodeResult utf8_encode(Rune c){
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

DecodeResult utf8_decode(Slice<Byte> s){
	DecodeResult res = {};
	Byte* buf = raw_data(s);
	Size slen = len(s);

	if(slen <= 0){ return DECODE_ERROR; }

	U8 first = buf[0];

	if((first & CONT) == 0){
		res.len = 1;
		res.codepoint |= first;
	}
	else if ((first & ~MASK2) == SIZE2 && slen >= 2){
		res.len = 2;
		res.codepoint |= (buf[0] & MASK2) << 6;
		res.codepoint |= (buf[1] & MASKX) << 0;
	}
	else if ((first & ~MASK3) == SIZE3 && slen >= 3){
		res.len = 3;
		res.codepoint |= (buf[0] & MASK3) << 12;
		res.codepoint |= (buf[1] & MASKX) << 6;
		res.codepoint |= (buf[2] & MASKX) << 0;
	}
	else if ((first & ~MASK4) == SIZE4 && slen >= 4){
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

bool iter_next(Utf8Iterator* it, Rune* r, I32* n){
	if(it->current >= len(it->data)){ return 0; }

	DecodeResult res = utf8_decode(slice_right(it->data, it->current));
	*r = res.codepoint;
	*n = res.len;

	if(res.codepoint == DECODE_ERROR.codepoint){
		*n = 1;
	}

	it->current += res.len;

	return 1;
}

bool iter_prev(Utf8Iterator* it, Rune* r, I32* len){
	if(it->current <= 0){ return false; }

	it->current -= 1;
	while(is_continuation_byte(it->data[it->current])){
		it->current -= 1;
	}

	DecodeResult res = utf8_decode(slice_right(it->data, it->current));
	*r = res.codepoint;
	*len = res.len;
	return true;
}

Rune iter_next(Utf8Iterator* it){
	I32 n = 0;
	Rune r;
	if(!iter_next(it, &r, &n)) return 0;
	return r;
}

Rune iter_prev(Utf8Iterator* it){
	I32 n = 0;
	Rune r;
	if(!iter_prev(it, &r, &n)) return 0;
	return r;
}



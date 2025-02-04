#include "base.hpp"

// Size String::rune_count() const {
// 	[[maybe_unused]] Rune r;
// 	[[maybe_unused]] i8 n;
// 	auto it = iterator();
// 	Size size = 0;
// 	while(it.next(&r, &n)) { size += 1; }
// 	return size;
// }
//
// String String::sub(Size from, Size to) const {
// 	bounds_check_assert(from <= to, "Improper slicing range");
// 	bounds_check_assert(from >= 0 && from <= _length, "Index to sub-string is out of bounds");
// 	bounds_check_assert(to >= 0 && to <= _length, "Index to sub-string is out of bounds");
// 	String s;
// 	s._length = to - from;
// 	s._data = &_data[from];
// 	return s;
// }
//
// constexpr Size MAX_CUTSET_LEN = 128;
//
// String String::trim(String cutset) const {
// 	String trimmed = this->trim_leading(cutset).trim_trailing(cutset);
// 	return trimmed;
// }
//
// utf8::Iterator String::iterator() const {
// 	return {
// 		.data = Slice<byte>::from_pointer((byte*)_data, _length),
// 		.current = 0,
// 	};
// }
//
// utf8::Iterator String::iterator_reversed() const {
// 	return {
// 		.data = Slice<byte>::from_pointer((byte*)_data, _length),
// 		.current = _length,
// 	};
// }
//
// String String::trim_leading(String cutset) const {
// 	debug_assert(cutset.size() <= MAX_CUTSET_LEN, "Cutset string exceeds MAX_CUTSET_LEN");
//
// 	Rune set[MAX_CUTSET_LEN] = {0};
// 	Size set_len = 0;
// 	Size cut_after = 0;
//
// 	decode_cutset: {
// 		Rune c; i8 n;
// 		auto iter = cutset.iterator();
//
// 		Size i = 0;
// 		while(iter.next(&c, &n) && i < MAX_CUTSET_LEN){
// 			set[i] = c;
// 			i += 1;
// 		}
// 		set_len = i;
// 	}
//
// 	strip_cutset: {
// 		Rune c; i8 n;
// 		auto iter = this->iterator();
//
// 		while(iter.next(&c, &n)){
// 			bool to_be_cut = false;
// 			for(Size i = 0; i < set_len; i += 1){
// 				if(set[i] == c){
// 					to_be_cut = true;
// 					break;
// 				}
// 			}
//
// 			if(to_be_cut){
// 				cut_after += n;
// 			}
// 			else {
// 				break; // Reached first Rune that isn't in cutset
// 			}
//
// 		}
// 	}
//
// 	return this->sub(cut_after, _length);
// }
//
// String String::trim_trailing(String cutset) const {
// 	debug_assert(cutset.size() <= MAX_CUTSET_LEN, "Cutset string exceeds MAX_CUTSET_LEN");
//
// 	Rune set[MAX_CUTSET_LEN] = {0};
// 	Size set_len = 0;
// 	Size cut_until = _length;
//
// 	decode_cutset: {
// 		Rune c; i8 n;
// 		auto iter = cutset.iterator();
//
// 		Size i = 0;
// 		while(iter.next(&c, &n) && i < MAX_CUTSET_LEN){
// 			set[i] = c;
// 			i += 1;
// 		}
// 		set_len = i;
// 	}
//
// 	strip_cutset: {
// 		Rune c; i8 n;
// 		auto iter = this->iterator_reversed();
//
// 		while(iter.prev(&c, &n)){
// 			bool to_be_cut = false;
// 			for(Size i = 0; i < set_len; i += 1){
// 				if(set[i] == c){
// 					to_be_cut = true;
// 					break;
// 				}
// 			}
//
// 			if(to_be_cut){
// 				cut_until -= n;
// 			}
// 			else {
// 				break; // Reached first Rune that isn't in cutset
// 			}
//
// 		}
// 	}
//
// 	return this->sub(0, cut_until);
// }




#include "base.hpp"

constexpr Size max_cutset_len = 64;

Utf8Iterator str_iterator(String s) {
	Utf8Iterator it = {
		.data = slice(raw_data(s), len(s)),
		.current = 0,
	};
	return it;
}

Utf8Iterator str_iterator_reversed(String s) {
	Utf8Iterator it = {
		.data = slice(raw_data(s), len(s)),
		.current = len(s),
	};
	return it;
}

String str_clone(String s, Allocator a){
	auto buf = make<Byte>(a, len(s) + 1);
	buf[len(buf) - 1] = 0;
	mem_copy_no_overlap(raw_data(buf), raw_data(s), len(s));
	return string_from_bytes(slice_left(buf, len(buf) - 1));
}

Size str_rune_count(String s) {
	[[maybe_unused]] Rune r;
	[[maybe_unused]] I32 n;
	auto it = str_iterator(s);
	Size size = 0;
	while(iter_next(&it, &r, &n)) { size += 1; }
	return size;
}

bool str_starts_with(String s, String prefix){
	if(len(prefix) == 0){ return true; }
	if(len(prefix) > len(s)){ return false; }
	I32 cmp = mem_compare(raw_data(s), raw_data(prefix), len(prefix));
	return cmp == 0;
}

bool str_ends_with(String s, String suffix){
	if(len(suffix) == 0){ return true; }
	if(len(suffix) > len(s)){ return false; }
	I32 cmp = mem_compare(raw_data(s) + len(s) - len(suffix), raw_data(suffix), len(suffix));
	return cmp == 0;
}

String str_trim(String s, String cutset) {
	String trimmed = str_trim_trailing(str_trim_leading(s, cutset), cutset);
	return trimmed;
}

String str_trim_leading(String s, String cutset) {
	debug_assert(len(cutset) <= max_cutset_len, "Cutset string exceeds max_cutset_len");

	Rune set[max_cutset_len] = {0};
	Size set_len = 0;
	Size cut_after = 0;

	/* Decode cutset */ {
		Rune c; I32 n;
		auto it = str_iterator(cutset);

		Size i = 0;
		while(iter_next(&it, &c, &n) && i < max_cutset_len){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	/* Strip cutset */ {
		Rune c; I32 n;
		auto it = str_iterator(s);

		while(iter_next(&it, &c, &n)){
			bool to_be_cut = false;
			for(Size i = 0; i < set_len; i += 1){
				if(set[i] == c){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_after += n;
			}
			else {
				break; // Reached first Rune that isn't in cutset
			}

		}
	}

	return slice_right(s, cut_after);
}

String str_trim_trailing(String s, String cutset) {
	debug_assert(len(cutset) <= max_cutset_len, "Cutset string exceeds max_cutset_len");

	Rune set[max_cutset_len] = {0};
	Size set_len = 0;
	Size cut_until = len(s);

	/* Decode cutset */ {
		Rune c; I32 n;
		auto it = str_iterator(cutset);

		Size i = 0;
		while(iter_next(&it, &c, &n) && i < max_cutset_len){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	/* Strip cutset */ {
		Rune c; I32 n;
		auto it = str_iterator_reversed(s);

		while(iter_prev(&it, &c, &n)){
			bool to_be_cut = false;
			for(Size i = 0; i < set_len; i += 1){
				if(set[i] == c){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_until -= n;
			}
			else {
				break; // Reached first Rune that isn't in cutset
			}

		}
	}

	return slice_left(s, cut_until);
}

Size find(String s, String pattern, Size start){
	bounds_check_assert(start < len(s), "Cannot begin searching after string length");
	if(len(pattern) > len(s)){ return -1; }
	else if(len(pattern) == 0){ return start; }

	auto source_p  = raw_data(s);
	auto pattern_p = raw_data(pattern);

	auto length = len(s) - len(pattern);

	for(Size i = start; i < length; i++){
		if(mem_compare(&source_p[i], pattern_p, len(pattern)) == 0){
			return i;
		}
	}

	return -1;
}

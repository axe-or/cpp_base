#include "base.hpp"

constexpr Size max_cutset_len = 64;

Utf8Iterator str_iterator(String s) {
	Utf8Iterator it = {
		.data = Slice(s.raw_data(), s.len()),
		.current = 0,
	};
	return it;
}

Utf8Iterator str_iterator_reversed(String s) {
	Utf8Iterator it = {
		.data = Slice(s.raw_data(), s.len()),
		.current = s.len(),
	};
	return it;
}

String str_clone(String s, Allocator a){
	auto buf = make<Byte>(a, s.len() + 1);
	[[unlikely]] if(buf.len() == 0){ return ""; }
	buf[buf.len() - 1] = 0;
	mem_copy_no_overlap(buf.raw_data(), s.raw_data(), s.len());
	return string_from_bytes(buf.slice_left(buf.len() - 1));
}

[[nodiscard]]
String str_concat(String s0, String s1, Allocator a){
	auto buf = make<Byte>(a, s0.len() + s1.len() + 1);

	[[unlikely]] if(buf.len() == 0){ return ""; }
	auto data = buf.raw_data();
	mem_copy_no_overlap(&data[0], s0.raw_data(), s0.len());
	mem_copy_no_overlap(&data[s0.len()], s1.raw_data(), s1.len());
	buf[buf.len() - 1] = 0;
	return string_from_bytes(buf.slice_left(buf.len() - 1));
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
	if(prefix.len() == 0){ return true; }
	if(prefix.len() > s.len()){ return false; }
	I32 cmp = mem_compare(s.raw_data(), prefix.raw_data(), prefix.len());
	return cmp == 0;
}

bool str_ends_with(String s, String suffix){
	if(suffix.len() == 0){ return true; }
	if(suffix.len() > s.len()){ return false; }
	I32 cmp = mem_compare(s.raw_data() + s.len() - suffix.len(), suffix.raw_data(), suffix.len());
	return cmp == 0;
}

String str_trim(String s, String cutset) {
	String trimmed = str_trim_trailing(str_trim_leading(s, cutset), cutset);
	return trimmed;
}

String str_trim_leading(String s, String cutset) {
	debug_assert(cutset.len() <= max_cutset_len, "Cutset string exceeds max_cutset_len");

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

	return s.slice_right(cut_after);
}

String str_trim_trailing(String s, String cutset) {
	debug_assert(cutset.len() <= max_cutset_len, "Cutset string exceeds max_cutset_len");

	Rune set[max_cutset_len] = {0};
	Size set_len = 0;
	Size cut_until = s.len();

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

	return s.slice_left(cut_until);
}

Size find(String s, String pattern, Size start){
	bounds_check_assert(start < s.len(), "Cannot begin searching after string length");
	if(pattern.len() > s.len()){ return -1; }
	else if(pattern.len() == 0){ return start; }

	auto source_p  = s.raw_data();
	auto pattern_p = pattern.raw_data();

	auto length = s.len() - pattern.len();

	for(Size i = start; i < length; i++){
		if(mem_compare(&source_p[i], pattern_p, pattern.len()) == 0){
			return i;
		}
	}

	return -1;
}

#include "base.hpp"

namespace strings {

constexpr Size max_cutset_len = 64;

Size rune_count(String s) {
	[[maybe_unused]] Rune r;
	[[maybe_unused]] I32 n;
	auto it = s.iterator();
	Size size = 0;
	while(it.next(&r, &n)) { size += 1; }
	return size;
}

String trim(String s, String cutset) {
	String trimmed = trim_trailing(trim_leading(s, cutset), cutset);
	return trimmed;
}

String trim_leading(String s, String cutset) {
	debug_assert(cutset.len() <= max_cutset_len, "Cutset string exceeds max_cutset_len");

	Rune set[max_cutset_len] = {0};
	Size set_len = 0;
	Size cut_after = 0;

	/* Decode cutset */ {
		Rune c; I32 n;
		auto iter = cutset.iterator();

		Size i = 0;
		while(iter.next(&c, &n) && i < max_cutset_len){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	/* Strip cutset */ {
		Rune c; I32 n;
		auto iter = s.iterator();

		while(iter.next(&c, &n)){
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

String trim_trailing(String s, String cutset) {
	debug_assert(cutset.len() <= max_cutset_len, "Cutset string exceeds max_cutset_len");

	Rune set[max_cutset_len] = {0};
	Size set_len = 0;
	Size cut_until = s.len();

	/* Decode cutset */ {
		Rune c; I32 n;
		auto iter = cutset.iterator();

		Size i = 0;
		while(iter.next(&c, &n) && i < max_cutset_len){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	/* Strip cutset */ {
		Rune c; I32 n;
		auto iter = s.iterator_reversed();

		while(iter.prev(&c, &n)){
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

}

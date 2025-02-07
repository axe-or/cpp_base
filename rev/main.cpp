#include "base.hpp"

using mem::Arena;
namespace str = strings;


int main(){
	print("Init");
	auto main_arena = Arena::create_virtual(5 * mem::MiB);
	String s = "Chup Chups";

	print(str::starts_with(s, "Chup"));
	print(str::ends_with(s, "Chups"));
	print(str::starts_with(s, ""));
	print(str::ends_with(s, ""));
	print(str::starts_with(s, s));
	print(str::ends_with(s, s));

	print(str::starts_with(s, "Chu"));
	print(str::ends_with(s, "ps"));

	String s2 = str::clone(s, &main_arena);
	print(s);
	print(s2);
	print(s == s2);

	print(str::find(s, "Chup", 1));

	return 0;
}

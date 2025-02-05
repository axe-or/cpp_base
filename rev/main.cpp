#include "base.hpp"

namespace str = strings;

int main(){
	String s = "  \t Chup Chups\n   \t\n";
	
	auto lead = str::trim_leading(s, " \t\r\n");
	auto trail = str::trim_trailing(s, " \t\r\n");
	auto both = str::trim(s, " \t\r\n");

	printf("REG:   \"%.*s\"\n", (int)s.len(), s.raw_data());
	printf("LEAD:  \"%*s\"\n", (int)lead.len(), lead.raw_data());
	printf("TRAIL: \"%.*s\"\n", (int)trail.len(), trail.raw_data());
	printf("BOTH:  \"%.*s\"\n", (int)both.len(), both.raw_data());

	return 0;
}

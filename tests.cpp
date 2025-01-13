#include "base.hpp"

namespace test {
static Atomic<i32> total = 0;
static Atomic<i32> failed = 0;
}
static inline
void expect_ex(bool pred, Source_Location loc){
	test::total.fetch_add(1);
	if(!pred){
		std::fprintf(stderr, "FAIL (%s:%d)", loc.filename, loc.line);
		test::failed.fetch_add(1);
	}
}
#define expect(Expr) expect_ex((Expr), this_location())

void test_slice(){
	auto allocator = mem::heap_allocator();
	auto s0 = allocator.make<int>(16);
	auto s1 = allocator.make<int>(16);
	defer(allocator.destroy(s0));
	defer(allocator.destroy(s1));

	for(isize i = 0; i < s0.size(); i++){
		s0[i] = i;
		s1[i] = i;
	}

	expect(s0.equals(s1));
	s1[5] = 69;
	expect(!s0.equals(s1));

	expect(s0.slice(0, 5).equals(s1.slice(0, 5)));
	expect(s0.slice(6, s0.size()).equals(s1.slice(6)));

	print(s0.slice(0, 5));
	print(s1.slice(6));

}


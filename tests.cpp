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


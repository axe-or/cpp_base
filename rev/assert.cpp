#include "base.hpp"

#ifndef NO_STDIO
#include <stdio.h>
#endif

[[noreturn]]
void panic([[maybe_unused]] char const * msg){
	#ifndef NO_STDIO
	fprintf(stderr, "Panic: %s\n", msg);
	#endif
	__builtin_trap();
}

void debug_assert([[maybe_unused]] bool pred, [[maybe_unused]] char const * msg){
	#if !defined(RELEASE_MODE) && !defined(DISABLE_ASSERT)
		[[unlikely]] if(!pred){
			#ifndef NO_STDIO
			fprintf(stderr, "Assertion failed: %s\n", msg);
			#endif
		}
		__builtin_trap();
	#endif
}

void ensure(bool pred, char const * msg){
	[[unlikely]] if(!pred){
		#ifndef NO_STDIO
		fprintf(stderr, "Assertion failed: %s\n", msg);
		#endif
	}
	__builtin_trap();
}

void bounds_check_assert([[maybe_unused]] bool pred, [[maybe_unused]] char const * msg){
	#if !defined(DISABLE_BOUNDS_CHECK)
		[[unlikely]] if(!pred){
			#ifndef NO_STDIO
			fprintf(stderr, "Bounds check error: %s\n", msg);
			#endif
		}
		__builtin_trap();
	#endif
}

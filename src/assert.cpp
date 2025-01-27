#include "base.hpp"
#include <stdio.h>

void debug_assert(bool pred, cstring msg, Source_Location const& loc){
	#if defined(ndebug) || defined(release_mode)
		(void)pred; (void)msg;
	#else
	[[unlikely]] if(!pred){
		fprintf(stderr, "(%s:%d) failed assert: %s\n", loc.filename, loc.line, msg);
		abort();
	}
	#endif
}

void ensure(bool pred, cstring msg, Source_Location const& loc){
	[[unlikely]] if(!pred){
		fprintf(stderr, "[%s:%d %s()] Failed assert: %s\n", loc.filename, loc.line, loc.caller_name, msg);
		abort();
	}
}

void bounds_check_assert(bool pred, cstring msg, Source_Location const& loc){
	#if defined(DISABLE_BOUNDS_CHECK)
		(void)pred; (void)msg;
	#else
	[[unlikely]]
	if(!pred){
		fprintf(stderr, "(%s:%d) Failed bounds check: %s\n", loc.filename, loc.line, msg);
		abort();
	}
	#endif
}

[[noreturn]] void panic(cstring msg, Source_Location const& loc){
	fprintf(stderr, "[%s:%d %s()] Panic: %s\n", loc.filename, loc.line, loc.caller_name, msg);
	abort();
}

[[noreturn]] void unimplemented(Source_Location const& loc){
	fprintf(stderr, "[%s:%d %s()] Unimplemented code.\n", loc.filename, loc.line, loc.caller_name);
	abort();
}


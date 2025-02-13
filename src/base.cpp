#include "platform_context.hpp"
#include "base.hpp"

#include "assert.cpp"
#include "memory.cpp"
#include "arena.cpp"
#include "utf8.cpp"
#include "strings.cpp"

#ifdef USE_MIMALLOC
	#include "mimalloc_allocator.cpp"
#endif

#include "virtual_memory.cpp"
#if defined(PLATFORM_OS_WINDOWS)
	#include "virtual_memory_windows.cpp"
#elif defined(PLATFORM_OS_LINUX)
	#include "virtual_memory_linux.cpp"
#endif


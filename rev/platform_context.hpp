#ifndef _platform_context_h_include_
#define _platform_context_h_include_

#if defined(__linux__)
	#define PLATFORM_OS_LINUX
#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_OS_LINUX
	#define PLATFORM_OS_WINDOWS
#else
	#error "Unsupported platform"
#endif

#endif /* Include guard */

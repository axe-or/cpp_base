#pragma once
//// Platform //////////////////////////////////////////////////////////////////
#if defined(TARGET_OS_WINDOWS)
	#define TARGET_OS_NAME "Windows"
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <stdlib.h>
	#undef min
	#undef max

#elif defined(TARGET_OS_LINUX)
	#define TARGET_OS_NAME "Linux"
	#define _XOPEN_SOURCE 800
#else
	#error "Platform macro `TARGET_OS_*` is not defined, this means you probably forgot to define it or this platform is not suported."
#endif

//// Essentials ////////////////////////////////////////////////////////////////
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdalign.h>
#include <stdnoreturn.h>

#include <stdatomic.h>
#include <tgmath.h>
#include <limits.h>
#include <float.h>

#define null NULL

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef unsigned int uint;
typedef uint8_t byte;

typedef ptrdiff_t isize;
typedef size_t    usize;

typedef uintptr_t uintptr;

typedef float  f32;
typedef double f64;

typedef char const * cstring;

// Swap bytes around, useful for when dealing with endianess
static inline
void swap_bytes_raw(byte* data, isize len){
	for(isize i = 0; i < (len / 2); i += 1){
		byte temp = data[i];
		data[i] = data[len - (i + 1)];
		data[len - (i + 1)] = temp;
	}
}

#define swap_bytes(Ptr) swap_bytes_raw((byte*)(Ptr), sizeof(*(Ptr)))

// This is to avoid conflict with stdlib's "abs()"
#define abs_val(X) (( (X) < 0ll) ? -(X) : (X))

#define min(A, B)  (((A) < (B)) ? (A) : (B))
#define max(A, B)  (((A) > (B)) ? (A) : (B))

#define clamp(Lo, X, Hi) min(max(Lo, X), Hi)

#define container_of(Ptr, Type, Member) \
	((Type *)(((void *)(Ptr)) - offsetof(Type, Member)))

#ifndef __cplusplus
#undef bool
typedef _Bool bool;
#define static_assert(Pred, Msg) _Static_assert(Pred, Msg)
#endif

static_assert(sizeof(f32) == 4 && sizeof(f64) == 8, "Bad float size");
static_assert(sizeof(isize) == sizeof(usize), "Mismatched (i/u)size");
static_assert(sizeof(void(*)(void)) == sizeof(void*), "Function pointers and data pointers must be of the same width");
static_assert(sizeof(void(*)(void)) == sizeof(uintptr), "Mismatched pointer types");
static_assert(CHAR_BIT == 8, "Invalid char size");

//// Spinlock //////////////////////////////////////////////////////////////////
#ifndef TARGET_DISABLE_ATOMICS
#define SPINLOCK_LOCKED 1
#define SPINLOCK_UNLOCKED 0

// The zeroed state of a spinlock is unlocked, to be effective across threads
// it's important to keep the spinlock outside of the stack and never mark it as
// a thread_local struct.
typedef struct {
	atomic_int _state;
} Spinlock;


// Enter a busy wait loop until spinlock is acquired(locked)
void spinlock_acquire(Spinlock* l);

// Try to lock spinlock, if failed, just move on. Returns if lock was locked.
bool spinlock_try_acquire(Spinlock* l);

// Release(unlock) the spinlock
void spinlock_release(Spinlock* l);

#define spinlock_guard(LockPtr, Scope) \
	do { spinlock_acquire(LockPtr); do { Scope } while(0); spinlock_release(LockPtr); } while(0)

#endif

//// Memory ////////////////////////////////////////////////////////////////////
typedef struct Mem_Allocator Mem_Allocator;

#define mem_new(Type, Num, Alloc) mem_alloc((Alloc), sizeof(Type) * (Num), alignof(Type));

// Helper to use with printf "%.*s"
#define fmt_bytes(buf) (int)((buf).len), (buf).data

enum Allocator_Op {
	Mem_Op_Query    = 0, // Query allocator's capabilities
	Mem_Op_Alloc    = 1, // Allocate a chunk of memory
	Mem_Op_Resize   = 2, // Resize an allocation in-place
	Mem_Op_Free     = 3, // Mark allocation as free
	Mem_Op_Free_All = 4, // Mark allocations as free
};

enum Allocator_Capability {
	Allocator_Alloc_Any = 1 << 0, // Can alloc any size
	Allocator_Free_Any  = 1 << 1, // Can free in any order
	Allocator_Free_All  = 1 << 2, // Can free all allocations
	Allocator_Resize    = 1 << 3, // Can resize in-place
	Allocator_Align_Any = 1 << 4, // Can alloc aligned to any alignment
};

// Memory allocator method
typedef void* (*Mem_Allocator_Func) (
	void * restrict impl,
	byte op,
	void* old_ptr,
	isize size, isize align,
	i32* capabilities
);

// Memory allocator interface
struct Mem_Allocator {
	Mem_Allocator_Func func;
	void* data;
};

// Set n bytes of p to value.
void mem_set(void* p, byte val, isize nbytes);

// Copy n bytes for source to destination, they may overlap.
void mem_copy(void* dest, void const * src, isize nbytes);

// Compare 2 buffers of memory, returns -1, 0, 1 depending on which buffer shows
// a bigger byte first, 0 meaning equality.
i32 mem_compare(void const * a, void const * b, isize nbytes);

// Copy n bytes for source to destination, they should not overlap, this tends
// to be faster then mem_copy
void mem_copy_no_overlap(void* dest, void const * src, isize nbytes);

// Align p to alignment a, this only works if a is a non-zero power of 2
uintptr align_forward_ptr(uintptr p, uintptr a);

// Align p to alignment a, this works for any positive non-zero alignment
uintptr align_forward_size(isize p, isize a);

// Get capabilities of allocator as a number, gets capability bit-set
u32 mem_query_capabilites(Mem_Allocator allocator);

// Allocate fresh memory, filled with 0s. Returns NULL on failure.
void* mem_alloc(Mem_Allocator allocator, isize size, isize align);

// Re-allocate memory in-place without changing the original pointer. Returns
// NULL on failure.
void* mem_resize(Mem_Allocator allocator, void* ptr, isize new_size);

// Free pointer to memory, includes alignment information, which is required for
// some allocators, freeing NULL is a no-op
void mem_free_ex(Mem_Allocator allocator, void* p, isize size, isize align);

// Free pointer to memory, freeing NULL is a no-op
void mem_free(Mem_Allocator allocator, void* p);

// Free all pointers owned by allocator
void mem_free_all(Mem_Allocator allocator);

// Re-allocate to new_size, first tries to resize in-place, then uses
// alloc->copy->free to attempt reallocation, returns null on failure
void* mem_realloc(Mem_Allocator allocator, void* ptr, isize old_size, isize new_size, isize align);

//// IO Interface //////////////////////////////////////////////////////////////
typedef struct IO_Stream IO_Stream;

enum Stream_Op {
	IO_Query = 0,
	IO_Read  = 1,
	IO_Write = 2,
};

enum Stream_Error {
	IO_Err_None          = 0,
	IO_Err_End_Of_Stream = -1,
	IO_Err_Unsupported   = -2,
	IO_Err_Memory_Error  = -3,
	IO_Err_Broken_Handle = -4,
};

enum Stream_Capability {
	IO_Stream_Read  = 1 << 0,
	IO_Stream_Write = 1 << 1,
};

// Function that represents a read or write operation. Returns number of bytes
// read into buf, returns a negative number indicating error if any.
typedef i64 (*IO_Func)(
	void* impl,
	byte op,
	byte* buf,
	isize buflen);

// Stream interface for IO operations.
struct IO_Stream {
	void*   data;
	IO_Func func;
};

// Read from stream into buf. Returns number of bytes read or (if negative) an error code.
i64 io_read(IO_Stream s, byte* buf, isize buflen);

// Write buf into stream. Returns number of bytes written or (if negative) an error code.
i64 io_write(IO_Stream s, byte* buf, isize buflen);

// Get stream object's capabilities
u8 io_capabilities(IO_Stream s);

//// Arena Allocator ///////////////////////////////////////////////////////////
typedef struct Mem_Arena Mem_Arena;

struct Mem_Arena {
	isize offset;
	isize capacity;
	uintptr last_allocation;
	byte* data;
};

// Initialize a memory arena with a buffer
void arena_init(Mem_Arena* a, byte* data, isize len);

// Deinit the arena
void arena_destroy(Mem_Arena *a);

// Resize arena allocation in-place, gives back same pointer on success, null on failure
void* arena_resize(Mem_Arena* a, void* ptr, isize new_size);

// Reset arena, marking all its owned pointers as freed
void arena_free_all(Mem_Arena* a);

// Allocate `size` bytes aligned to `align`, return null on failure
void *arena_alloc(Mem_Arena* a, isize size, isize align);

// Get arena as a conforming instance to the allocator interface
Mem_Allocator arena_allocator(Mem_Arena* a);

//// Pool Allocator ////////////////////////////////////////////////////////////
typedef struct Mem_Pool Mem_Pool;
typedef struct Mem_Pool_Node Mem_Pool_Node;

struct Mem_Pool {
	byte* data;
	isize capacity;
	isize node_size;
	Mem_Pool_Node* free_list;
};

struct Mem_Pool_Node {
	Mem_Pool_Node* next;
};

// Initialize pool allocator with nodes of a particular size and alignment.
// Returns success status
bool pool_init(Mem_Pool* pool, byte* data, isize len, isize node_size, isize node_alignment);

// Mark all the pool's allocations as freed
void pool_free_all(Mem_Pool* pool);

// Mark specified pointer returned by `pool_alloc` as free again
void pool_free(Mem_Pool* pool, void* ptr);

// Allocate one node from pool, returns null on failure
void* pool_alloc(Mem_Pool* pool);

// Get pool as a conforming instance to the allocator interface
Mem_Allocator pool_allocator(Mem_Pool* pool);

//// UTF-8 /////////////////////////////////////////////////////////////////////
typedef i32 rune;
typedef struct UTF8_Encode_Result UTF8_Encode_Result;
typedef struct UTF8_Decode_Result UTF8_Decode_Result;
typedef struct UTF8_Iterator UTF8_Iterator;

// UTF-8 encoding result, a len = 0 means an error.
struct UTF8_Encode_Result {
	byte bytes[4];
	i8 len;
};

// UTF-8 encoding result, a len = 0 means an error.
struct UTF8_Decode_Result {
	rune codepoint;
	i8 len;
};

// The error rune
#define UTF8_ERROR ((rune)(0xfffd))

// The error rune, byte encoded
static const UTF8_Encode_Result UTF8_ERROR_ENCODED = {
	.bytes = {0xef, 0xbf, 0xbd},
	.len = 0,
};

// Encode a unicode rune
UTF8_Encode_Result utf8_encode(rune c);

// Decode a rune from a UTF8 buffer of bytes
UTF8_Decode_Result utf8_decode(byte const* data, isize len);

// Allows to iterate a stream of bytes as a sequence of runes
struct UTF8_Iterator {
	byte const* data;
	isize data_length;
	isize current;
};

// Steps iterator forward and puts rune and Length advanced into pointers,
// returns false when finished.
bool utf8_iter_next(UTF8_Iterator* iter, rune* r, i8* len);

// Steps iterator backward and puts rune and its length into pointers,
// returns false when finished.
bool utf8_iter_prev(UTF8_Iterator* iter, rune* r, i8* len);

//// Strings ///////////////////////////////////////////////////////////////////
typedef struct String String;

#define str_lit(CstrLit) (String){ .data = (byte const*)(CstrLit), .len = (sizeof(CstrLit) - 1) }

struct String {
	byte const * data;
	isize _length;
};

static inline
isize cstring_len(cstring cstr){
	static const isize CSTR_MAX_LENGTH = (~(u32)0) >> 1;
	isize size = 0;
	for(isize i = 0; i < CSTR_MAX_LENGTH && cstr[i] != 0; i += 1){
		size += 1;
	}
	return size;
}

// Create substring from a cstring
String str_from(cstring data);

// Create substring from a piece of a cstring
String str_from_range(cstring data, isize start, isize length);

// Create substring from a raw slice of bytes
String str_from_bytes(byte const* data, isize length);

// Get a sub string, starting at `start` with `length`
String str_sub(String s, isize start, isize length);

// Get how many codeponits are in a string
isize str_codepoint_count(String s);

// Get the byte offset of the n-th codepoint
isize str_codepoint_offset(String s, isize n);

// Clone a string
String str_clone(String s, Mem_Allocator allocator);

// Destroy a string
void str_destroy(String s, Mem_Allocator allocator);

// Concatenate 2 strings
String str_concat(String a, String b, Mem_Allocator allocator);

// Check if 2 strings are equal
bool str_eq(String a, String b);

// Trim leading codepoints that belong to the cutset
String str_trim_leading(String s, String cutset);

// Trim trailing codepoints that belong to the cutset
String str_trim_trailing(String s, String cutset);

// Trim leading and trailing codepoints
String str_trim(String s, String cutset);

// Check if string starts with a prefix
bool str_starts_with(String s, String prefix);

// Check if string ends with a suffix
bool str_ends_with(String s, String suffix);

// Get an utf8 iterator from string
UTF8_Iterator str_iterator(String s);

// Get an utf8 iterator from string, already at the end, to be used for reverse iteration
UTF8_Iterator str_iterator_reversed(String s);

// Is string empty?
bool str_empty(String s);

//// Source Location ///////////////////////////////////////////////////////////
typedef struct Source_Location Source_Location;
typedef enum Logger_Option Logger_Option;

struct Source_Location {
    cstring filename;
    cstring caller_name;
    i32 line;
};

#define this_location() this_location_()

#define this_location_() (Source_Location){ \
    .filename = __FILE__, \
    .caller_name = __func__, \
    .line = __LINE__, \
}

//// Assert ////////////////////////////////////////////////////////////////////
// Crash if `pred` is false, this is disabled in non-debug builds
void debug_assert_ex(bool pred, cstring msg, Source_Location loc);

#if defined(NDEBUG) || defined(RELEASE_MODE)
#define debug_assert(Pred, Msg) ((void)0)
#else
#define debug_assert(Pred, Msg) debug_assert_ex(Pred, Msg, this_location())
#endif

// Crash if `pred` is false, this is always enabled
void panic_assert_ex(bool pred, cstring msg, Source_Location loc);

#define panic_assert(Pred, Msg) panic_assert_ex(Pred, Msg, this_location())

// Crash the program with a fatal error
noreturn void panic(char * const msg);

// Crash the program due to unimplemented code paths, this should *only* be used
// during development
noreturn void unimplemented();

//// Logger ////////////////////////////////////////////////////////////////////
typedef struct Logger Logger;

typedef i32 (*Logger_Func)(
    void* impl,
    String message,
	u32 options,
    u8 level,
    Source_Location location
);

enum Logger_Option {
	Logger_Date      = 1 << 0,
	Logger_Time      = 1 << 1,
	Logger_Caller    = 1 << 2,
	Logger_Filename  = 1 << 3,
	Logger_Use_Color = 1 << 4,
};

enum Log_Level {
    Log_Debug = 0,
    Log_Info  = 1,
    Log_Warn  = 2,
    Log_Error = 3,
    Log_Fatal = 4,
};

static const cstring log_level_map[] = {
    [Log_Debug] = "DEBUG",
    [Log_Info]  = "INFO",
    [Log_Warn]  = "WARN",
    [Log_Error] = "ERROR",
    [Log_Fatal] = "FATAL",
};

struct Logger {
    void* impl;
    Logger_Func log_func;
	u32 options;
};

// Log(explicit) using string
i32 log_ex_str(Logger l, String message, Source_Location loc, u8 level_n);

// Log(explicit) using cstring
i32 log_ex_cstr(Logger l, cstring message, Source_Location loc, u8 level_n);

// Log (explicit), generic version
#define log_ex(LoggerObj, Msg, Loc, Level) \
    _Generic((Msg),  \
    cstring: log_ex_cstr, \
    char* : log_ex_cstr, \
    String: log_ex_str)(LoggerObj, Msg, Loc, Level)

// Log Helper (info)
#define log_info(LoggerObj, Msg) log_ex((LoggerObj), (Msg), this_location(), Log_Info)

// Log Helper (debug)
#define log_debug(LoggerObj, Msg) log_ex((LoggerObj), (Msg), this_location(), Log_Debug)

// Log Helper (warn)
#define log_warn(LoggerObj, Msg) log_ex((LoggerObj), (Msg), this_location(), Log_Warn)

// Log Helper (error)
#define log_error(LoggerObj, Msg) log_ex((LoggerObj), (Msg), this_location(), Log_Error)

// Log Helper (fatal)
#define log_fatal(LoggerObj, Msg) log_ex((LoggerObj), (Msg), this_location(), Log_Fatal)

//// Console Logger ////////////////////////////////////////////////////////////
#ifndef TARGET_OS_FREESTANDING
typedef struct Console_Logger Console_Logger;

struct Console_Logger {
	// TODO: allow std/stderr distinctino
	Mem_Allocator parent_allocator;
};

// Create a logger that prints to stdout
Console_Logger* log_create_console_logger(Mem_Allocator allocator);

// Destroy console logger
void log_destroy_console_logger(Console_Logger* cl);

// Get the Logger interface from a console logger
Logger log_console_logger(Console_Logger* cl, u32 options);
#endif

//// String Builder ////////////////////////////////////////////////////////////
typedef struct String_Builder String_Builder;

struct String_Builder {
	byte* data;
	isize len;
	isize cap;
	Mem_Allocator allocator;
};

// Initialize a string builder, returns false on failure
bool sb_init(String_Builder* sb, Mem_Allocator allocator, isize initial_cap);

// Free current string builder buffer
void sb_destroy(String_Builder* sb);

// Append buffer of bytes to the end of builder. Returns < 0 if an error occours
// and number of bytes added otherwhise
isize sb_append_bytes(String_Builder* sb, byte const* buf, isize nbytes);

// Build owned string, resetting internal variables of builder
String sb_build(String_Builder* sb);

// Append utf-8 string to builder
isize sb_append_str(String_Builder* sb, String s);

// Append encoded rune to string builder
isize sb_append_rune(String_Builder* sb, rune r);

// Reset builder's buffer and length, does not free memory
void sb_clear(String_Builder* sb);

//// Time //////////////////////////////////////////////////////////////////////
typedef struct Time_Point Time_Point;

// Difference between 2 time points (in nanoseconds)
// Note that a 64bit signed integer can handle around +-292 years.
typedef i64 Time_Duration;

// UNIX Epoch scaled to nanosecond precision
struct Time_Point {
	i64 _nsec;
};

// Get current system Time_Point
Time_Point time_now();

// Sleep current thread for a duration
void time_sleep(Time_Duration d);

// Time since timepoint
Time_Duration time_since(Time_Point p);

// Time difference
Time_Duration time_point_diff(Time_Point a, Time_Point b);

// Time difference
Time_Duration time_duration_diff(Time_Duration a, Time_Duration b);

// Time constants, as a multiple of durations
#define time_nanosecond  ((i64)1ll)
#define time_microsecond ((i64)1000ll)
#define time_millisecond ((i64)1000000ll)
#define time_second      ((i64)1000000000ll)
#define time_minute      ((i64)(60ll * 1000000000ll))
#define time_hour        ((i64)(3600ll * 1000000000ll))

#define time_diff(A, B) _Generic((A), \
	Time_Duration: time_duration_diff, \
	Time_Point: time_point_diff)(A, B)

//// LibC Allocator ////////////////////////////////////////////////////////////
#ifndef TARGET_OS_FREESTANDING
// Wrapper around libc's aligned_alloc() and free()
Mem_Allocator libc_allocator();

#endif


# c-utilities
A collection of C utilities: aligned memory allocation with multiple backends, error handling, dynamic arrays, clamp functions, OpenGL helpers and more

# aligned_memory
Portable C library for aligned memory allocation.  
Support Windows, POSIX, C11 and fallback with a malloc.

# assert_m
Flexible assert macros:
- `assert_m(condition, message)` - assertion with a string message
- `assert_mf(condition, format, ...)` - assertion with format (require C99 or GCC/clang extensions)
- `static_assert_m(condition, message)` - compile-time checks (require C11 with correct text or fallback)
Disabled when NDEBUG is defined.

# clamp_values
Inline clamping functions  
Supported types: int64_t, uint64_t, size_t, float, double, long double  
For narrower integers (int32 and others) you can safely cast to the corresponding supported type (or there will be implicit conversion which isn't recommended)  

# cleanup_register
Register a cleanup function with optional argument to be called at program exit via `atexit`.  
- Supports **one** registered function with a single void * argument. Subsequent calls with a new function pointer are ignored
- The **argument pointer** can be updated at any time by calling function without a function_pointer
- At least one of the pointers must be non-`NULL` (enforced in debug builds)
- Requires C99.

# exit_print
Print an error message to stderr and exit with `EXIT_FAILURE`  
- `ep_exit_print(format_pointer, ...)` - prints formatted message and exit  
- `ep_exit_print_free(free_flag, format_pointer, ...)` - prints formatted message, frees `format_pointer` if `free_flag` is true and `format_pointer` is not `NULL` and exit  
`format_pointer` must be non-`NULL` (enforced in debug builds)

License: Apache 2.0, Version 2.0

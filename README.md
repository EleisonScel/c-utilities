# c-utilities
A collection of C utilities: aligned memory allocation with multiple backends, error handling, dynamic arrays, clamp functions, OpenGL helpers and more

# aligned_memory
Portable C library for aligned memory allocation.  
Supports Windows, POSIX, C11 and fallback using standard malloc.
> `am_aligned_realloc(pointer, 0)` is **not** equivalent to `am_aligned_free(pointer)` - it returns `NULL` without releasing the memory block.  
> In C11 the size passed to `am_aligned_malloc` must be a multiple of alignment. The **library automatically rounds up** the size when using the C11 backend  
> In C23 this requirement is removed. To avoid unnecessary overhead, the library does **not** round the size when compiled for C23 support. If your code relies on rounding, ensure the size is a multiple of alignment explicitly.  

# assert_m
Flexible assert macros:
- `assert_m(condition, message)` - assertion with a string message
- `assert_mf(condition, format, ...)` - assertion with format (requires C99 or GCC/clang extensions)
- `static_assert_m(condition, message)` - compile-time checks (requires C11 with a string literal or C89 but text will not be accurate)  
> `assert_m` and `assert_mf` are disabled when `NDEBUG` is defined.
> `static_assert_m` is **not** affected by `NDEBUG`

# clamp_values
Inline clamping functions  
Supported types: int64_t, uint64_t, size_t, float, double, long double  
For narrower integers (for example `int32_t`) you can safely cast to the corresponding supported type; otherwise an implicit conversion will occur, which isn't recommended  

# cleanup_register
Register a cleanup function with optional argument to be called at program exit via `atexit`.  
- Supports **one** registered function with a single void * argument. Subsequent calls with a new function pointer are ignored
- The **argument pointer** can be updated at any time by calling function without a function pointer
- At least one of the pointers must be non-`NULL` (enforced in debug builds)
- Requires C99.

# exit_print
Print an error message to stderr and exit with `EXIT_FAILURE`  
- `ep_exit_print(format_pointer, ...)` - prints a formatted message and exits  
- `ep_exit_print_free(free_flag, format_pointer, ...)` - prints formatted message, frees `format_pointer` if `free_flag` is true and `format_pointer` is not `NULL` and then exit  
`format_pointer` must be non-`NULL` (enforced in debug builds)

---

**License**: Apache 2.0

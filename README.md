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

License: Apache 2.0, Version 2.0

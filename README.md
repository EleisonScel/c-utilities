# c-utilities
A collection of C utilities: aligned memory allocation with multiple backends, error handling, dynamic arrays, clamp functions, OpenGL helpers and more.

---

## Modules

### aligned_memory
Portable C library for aligned memory allocation.  
Supports Windows, POSIX, C11 and fallback using standard malloc.
> `am_aligned_realloc(pointer, 0)` is **not** equivalent to `am_aligned_free(pointer)` - it returns `NULL` without releasing the memory block.  
> In C11 the size passed to `am_aligned_malloc` must be a multiple of alignment. The **library automatically rounds up** the size when using the C11 backend.  
> In C23 this requirement is removed. To avoid unnecessary overhead, the library does **not** round the size when compiled for C23 support. If your code relies on rounding, ensure the size is a multiple of alignment explicitly.  

### assert_m
Flexible assert macros:
- `assert_m(condition, message)` - assertion with a string message.  
- `assert_mf(condition, format, ...)` - assertion with format (requires C99 or GCC/clang extensions).  
- `static_assert_m(condition, message)` - compile-time checks (requires C11 with a string literal or C89 but text will not be accurate).  
> `assert_m` and `assert_mf` are disabled when `NDEBUG` is defined.
> `static_assert_m` is **not** affected by `NDEBUG`.

### clamp_values
Inline clamping functions.  
Supported types: int64_t, uint64_t, size_t, float, double, long double.  
For narrower integers (for example `int32_t`) you can safely cast to the corresponding supported type; otherwise an implicit conversion will occur, which isn't recommended.  

### cleanup_register
Register a cleanup function with optional argument to be called at program exit via `atexit`.  
- Supports **one** registered function with a single void * argument. Subsequent calls with a new function pointer are ignored.
- The **argument pointer** can be updated at any time by calling function without a function pointer.
- At least one of the pointers must be non-`NULL` (enforced in debug builds).
- Requires C99.

### exit_print
Print an error message to stderr and exit with `EXIT_FAILURE`.  
- `ep_exit_print(format_pointer, ...)` - prints a formatted message and exits.  
- `ep_exit_print_free(free_flag, format_pointer, ...)` - prints formatted message, frees `format_pointer` if `free_flag` is true and `format_pointer` is not `NULL` and then exit.  
`format_pointer` must be non-`NULL` (enforced in debug builds).  

### dynamic_array
Dynamic array utilities with automatic resizing and memory management.
- `da_dynamic_array_ensure_capacity` - ensures the array has at least `needed` capacity, growing by a factor of 1.5; if that allocation fails, it attempts to allocate exactly `needed` elements and if both allocations fail, returns `false`.
- `da_dynamic_array_ensure_capacity_list` - same as `da_dynamic_array_ensure_capacity` but operates on multiple dynamic arrays simultaneously (all resized to the same capacity) to simplify consistent resizing of related arrays. Requires an array of `DA_Dynamic_Array_List` structures, each specifying the data pointer and element size for the corresponding array.
- `da_dynamic_array_shrink` - shrinks capacity to fit `amount` elements (if `amount` smaller than `base_amount`, capacity is set to `base_amount`). If both `amount` and `base_amount` are `0`, the array is freed and capacity is zeroed.
- `da_dynamic_array_free` - frees the array and nullifies the pointer, size, and capacity.  
> All functions perform bounds checking and handle allocation failures gracefully.  
> Requires C99.

### write_out_error_message
Stack-like error message storage and retrieval.
- `woem_push` - formats and stores an error message;
- `woem_push_raw` - stores a pre-allocated error message;
- `woem_pop` - retrieves the most recent error message and removes it from storage; returns NULL if none exists and also returns flag indicating memory ownership. 
- `woem_shrink` - shrinks the dynamic storage to the exact number of messages (optimises memory usage).
- `woem_clear` - frees all stored messages and resets the storage.
> **Memory ownership**  
> - `woem_push` allocate memory internally.  
> - `woem_push_raw` takes ownership of the passed pointer.  
> - `woem_pop` returns a flag and a pointer, if the flag is `true` the caller is responsible for calling `free` on returned pointer.  
> - `woem_clear` frees all remaining messages.  

### gl_utils
OpenGL error checking.  
- `gl_errors_clean` - clear all pending OpenGL errors.  
- `gl_errors_check` - check for errors, returns `true` if none, otherwise stores errors via `woem_push`.  
- `gl_get_error_string` - returns a string description for a given OpenGL error code.  
> Requires OpenGL and GLEW.  

### gl_wrappers
Safe wrappers for all OpenGL functions that may set an error state; zero overhead in release builds.
- `glCreateShader_wrapped`
- `glShaderSource_wrapped`
- `glCompileShader_wrapped`
- `glCreateProgram_wrapped`
- `glAttachShader_wrapped`
- `glLinkProgram_wrapped`
- `glDeleteShader_wrapped`
- `glDeleteProgram_wrapped`
> Functions return `bool`. In release builds they always `true`; in debug they validate arguments and return result of OpenGL error checking.  
> Requires OpenGL and GLEW.  

### create_shader_program
Create OpenGL shader program from vertex and fragment sources.
- `csp_create_shader_program` - create a shader program from a single fragment and single vertex shader source code string.
- `csp_create_shader_program_many_sources` - creates a program from one or more vertex and fragment source strings.

---

**License**: Apache 2.0

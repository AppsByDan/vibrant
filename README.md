# vibrant

vibrant is a single-header C/C++ color conversion library with a parser for CSS-like color strings.

It converts colors from various color spaces (HSL, HWB, LCH, LAB, Oklch, Oklab) and formats (Hex, CSS Names) into sRGB.

## Features

*   **Single Header**: Easy to integrate into any project.
*   **CSS Color Parser**: Parses CSS color strings including:
    *   Hex codes (`#fff`, `#ffffff`, etc.)
    *   Named colors (`red`, `cornflowerblue`, etc.)
    *   Functional notation (`rgb()`, `hsl()`, `hwb()`, `lch()`, `lab()`, `oklch()`, `oklab()`)
    *   Modern CSS syntax support (space-separated components, alpha via `/`).
*   **Flexible Output**: Receive color data as `uint8_t` [0-255] or floating point [0-1], either by value or directly into your own data structures.
*   **Configurable**: Options for double precision and static linkage.
*   **Zero Allocations**: The library does not perform dynamic memory allocation.

## Usage

As a single-header library, you must define `VIBRANT_IMPLEMENTATION` in exactly one source file before including the header.

```c
#include <stdio.h>

#define VIBRANT_IMPLEMENTATION
#include "vibrant.h"

int main() {
  // Initialize a receiver to get RGBA values as uint8_t (0-255)
  vbt_recv_t recv = vbt_recv_init();

  // Parse a CSS color string
  int res = vbt_parse_z("hsl(180.0, 50%, 50%)", &recv);

  if (res == VBT_SUCCESS) {
    printf("R: %u, G: %u, B: %u, A: %u\n",
      recv.u.val.u8.r, recv.u.val.u8.g, recv.u.val.u8.b, recv.u.val.u8.a);
  }

  return 0;
}
```

### Linking

vibrant depends on `math.h`. On Linux, you may need to link with `-lm`.

## API Overview

### Parsing

*   `vbt_parse(const char* value, size_t len, vbt_recv_t* recv)`: Parse a string with a known length.
*   `vbt_parse_z(const char* value, vbt_recv_t* recv)`: Parse a null-terminated string.

### Manual Conversion

You can also use specific conversion functions directly:

*   `vbt_rgb(...)`
*   `vbt_hsl(...)`
*   `vbt_hwb(...)`
*   `vbt_lch(...)`
*   `vbt_lab(...)`
*   `vbt_oklch(...)`
*   `vbt_oklab(...)`

### Receiving Values

The `vbt_recv_t` struct determines how the output color is stored. You can initialize it to receive values by value or by reference.

**By Value (Default):**

```c
vbt_recv_t recv = vbt_recv_init(); // Defaults to VBT_RECV_VAL_U8
// Access via recv.u.val.u8.r, etc.
```

**By Reference (Float):**

Useful for writing directly into your own math library's vector types.

```c
float r, g, b, a;
vbt_recv_t recv = vbt_recv_init_ref_f32(&r, &g, &b, &a);
vbt_parse_z("red", &recv);
// r, g, b, a are now populated with 1.0, 0.0, 0.0, 1.0
```

## Configuration

Define these macros before including `vibrant.h` to configure the library:

*   `VIBRANT_NO_PARSE`: Disables the parsing functionality, leaving only the conversion logic. Reduces binary size if parsing isn't needed.
*   `VIBRANT_STATIC`: Declares functions with `static` linkage (internal) instead of `extern`.
*   `VIBRANT_DOUBLE_PRECISION`: Uses `double` instead of `float` for internal calculations and output types.

# Testing

To run the tests:

```
cd test
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
(cd test && ctest .)
```

# License

This project is dual-licensed under the MIT license and the Apache License (Version 2.0). You may choose either license at your option.

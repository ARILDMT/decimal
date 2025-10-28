# Decimal Library

A C implementation of a decimal data type for precise financial calculations.

## Overview

This library implements a decimal type that eliminates floating-point arithmetic errors, making it suitable for financial calculations where precision is critical. The decimal type supports numbers from -79,228,162,514,264,337,593,543,950,335 to +79,228,162,514,264,337,593,543,950,335.

## Features

### Arithmetic Operations
- **Addition** (`add`) - Add two decimal numbers
- **Subtraction** (`sub`) - Subtract one decimal from another
- **Multiplication** (`mul`) - Multiply two decimal numbers
- **Division** (`div`) - Divide one decimal by another

### Comparison Operations
- `is_less` - Check if first number is less than second
- `is_less_or_equal` - Check if first number is less than or equal to second
- `is_greater` - Check if first number is greater than second
- `is_greater_or_equal` - Check if first number is greater than or equal to second
- `is_equal` - Check if two numbers are equal
- `is_not_equal` - Check if two numbers are not equal

### Conversion Functions
- `from_int_to_decimal` - Convert integer to decimal
- `from_float_to_decimal` - Convert float to decimal
- `from_decimal_to_int` - Convert decimal to integer
- `from_decimal_to_float` - Convert decimal to float

### Rounding Functions
- `floor_decimal` - Round toward negative infinity
- `round_decimal` - Round to nearest integer
- `truncate_decimal` - Remove fractional part
- `negate_decimal` - Multiply by -1

## Binary Representation

The decimal type is represented as a structure with a 4-element array of 32-bit integers:

```c
typedef struct {
  int bits[4];
} decimal;
```

- `bits[0]`, `bits[1]`, `bits[2]` - Store the low, middle, and high 32 bits of the 96-bit integer
- `bits[3]` - Contains the scale factor and sign:
  - Bits 0-15: Unused (must be zero)
  - Bits 16-23: Scale factor (0-28), indicates power of 10 divisor
  - Bits 24-30: Unused (must be zero)
  - Bit 31: Sign (0 = positive, 1 = negative)

## Building

```bash
cd src
make all        # Build the library
make test       # Build and run tests
make gcov_report # Generate code coverage report
make clean      # Clean build artifacts
```

## Usage Example

```c
#include "decimal.h"

int main() {
    decimal a, b, result;

    // Convert integers to decimal
    from_int_to_decimal(100, &a);
    from_int_to_decimal(50, &b);

    // Perform addition
    if (add(a, b, &result) == ARITHMETIC_OK) {
        int value;
        from_decimal_to_int(result, &value);
        printf("Result: %d\n", value); // Output: 150
    }

    return 0;
}
```

## Return Codes

### Arithmetic Operations
- `0` (ARITHMETIC_OK) - Success
- `1` (ARITHMETIC_BIG) - Number too large or infinity
- `2` (ARITHMETIC_SMALL) - Number too small or negative infinity
- `3` (ARITHMETIC_DIV_BY_ZERO) - Division by zero

### Comparison Operations
- `0` - FALSE
- `1` - TRUE

### Conversion Functions
- `0` - Success
- `1` - Conversion error

## Technical Details

- **Language**: C11 standard
- **Compiler**: GCC with flags `-Wall -Wextra -Werror`
- **Testing**: Unit tests using Check framework
- **Coverage**: Minimum 80% code coverage
- **Rounding**: Banker's rounding (round to nearest even) for overflow scenarios

## Project Structure

```
decimal/
├── src/
│   ├── decimal.h          # Header file with type and function declarations
│   ├── arithmetic.c       # Arithmetic operations implementation
│   ├── compare.c          # Comparison operations implementation
│   ├── utils.c            # Utility and conversion functions
│   ├── test_decimal.c     # Unit tests
│   └── Makefile          # Build configuration
└── README.md             # This file
```

## License

See LICENSE file for details.

## Notes

- The implementation uses banker's rounding for numbers that don't fit in the mantissa
- Trailing zeros can be preserved or removed (except in `truncate_decimal`)
- The `__int128` type is not used in this implementation
- Binary representation follows the standard decimal format with 96-bit integer storage

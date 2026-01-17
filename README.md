# Partial Byte Buffer

## Why

This buffer supports writing and reading data at bit-level sizes. This is useful when your data never uses up a number of full bytes in their binary representation. For example, 33 can be represented with only 7 bits (0100001, one bit for sign), rather than a full byte. We can easily find some practical scenarios like value ranges of geographic coordinates (latitude/longitude), or altitude/elevation measurements, temperature readings.

Take the longitude coordinates on Earth for example. The values range from -180.00000° to 180.00000° (5 decimal places). Using the standard IEEE-754 double-precision floats needs a fixed number of 11 exponent bits and 52 mantissa bits. However, 6 exponent bits and 25 mantissa bits are actually enough for values at 5 decimal digit precision. This helps to save around 50% of normally used memory per one value.

This space efficiency can be accumulated when you store values of the same type in one sequence.

## Supported Operations

The buffer supports write and read methods that allow input data as a byte or integer (32 or 64 bits). The key argument is the number of valid bits related to the data to write to or read from the buffer. These are the bits that will be extracted from the input data and written to the buffer, or the bits of data to read from the buffer starting at the current position.

Example: Write 7 bits of number 33 into a partial byte buffer.
```c
#include "partial_byte_buffer.h"

partial_byte_buffer *pbb = pbb_create(2);   // Create a partial byte buffer with size 2
pbb_write_int(pbb, 33, 7);                  // Write 7 bits of an int
pbb_write_byte(pbb, 7, 3);                  // Write 3 bits of a byte
```

## Expandable Capacity

The buffer can be allocated with an initial capacity and has the ability to grow this size when the data to write exceeds the current maximum space.

There are two capacity growth strategies: **Grow By Double** or **Grow By Half**, which multiply the current size by 2 or 1.5, respectively. This expansion behavior is triggered before an actual write is executed, when the current bits plus the bits to write exceeds the capacity.

## Unit Test

Test script ```test.sh``` supports two options:

- `-t`: Specify which test file to run.

- `-m`: Capacity expansion mode with 0 is **Double** and 1 is **Grow By Half** strategy.

Example: Running tests for the Grow By Half strategy **must** use `-m 1`.
```
./test.sh -t ./pbb/test/PartialByteBufferCapacityOneAndHalfTest.cpp -m 1
```

## TODOs

## References
1. Circular Buffer - https://en.wikipedia.org/wiki/Circular_buffer
1. Linux Socket Buffer -  https://dev.to/amrelhusseiny/linux-networking-part-1-kernel-net-stack-180l 
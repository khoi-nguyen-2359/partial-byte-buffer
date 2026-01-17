# Partial Byte Buffer

## Why

This buffer supports writing and reading data at bit-level sizes. This is particularly useful when the values your data consists of never use up a number of full bytes in their binary representation. For example, the number 33 can be represented in only 7 bits (0100001, one bit for sign), rather than a full byte. We can easily find some practical scenarios like value ranges of GPAs (0.0 - 4.5), human heights (0 - a few hundred cm), or geographic coordinates (-180 to 180).

Take the longitude coordinates on Earth for example. The values range from -180.00000° to 180.00000° (5 decimal places). Using the standard IEEE-754 double-precision floats needs a fixed number of 11 exponent bits and 52 mantissa bits. However, 6 exponent bits and 25 mantissa bits are actually enough for values at 5 decimal digit precision. This helps to save around 50% of normally used memory per one value. See the [Range Test Cases](pbb/test/FloatResizerRangeTest.cpp) for more examples.

This space efficiency can be accumulated when you store values of the same type in one sequence.

## Supported Operations

The buffer supports write and read methods that allow input data as a byte or integer (32 or 64 bits). The key argument is the number of valid bits related to the data to write to or read from the buffer. These are the bits that will be extracted from the input data and written to the buffer, or the bits of data to read from the buffer starting at the current position.

## Extensible Capacity

The buffer can be allocated with an initial capacity and has the ability to grow when the data to write exceeds the current maximum space.

There are two capacity growth strategies: **Grow By Double** or **Grow By Half**, which multiply the current size by 2 or 1.5, respectively. This extension behavior is triggered before an actual write is executed, when the current bits plus the bits to write exceeds the capacity.

## Floating-point Compression

## Unit Test

Test script ```test.sh``` supports two options:

- Test file: use ```-t path/to/test/file.cpp``` to specify which test file to run.
- Capacity extension mode: use ```-m 0|1``` with 0 is **Double** and 1 is **Grow By Half** strategy.


## TODOs


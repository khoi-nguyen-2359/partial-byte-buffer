Partial Byte Buffer
---
A growable buffer type that supports writing and reading data at bit-level sizes.

1. Why

This buffer supports write and read data at sub-byte level. This is particularly useful when the range of your data will never use up a number of full bytes in the representation. Especially, when you are required to store those values in a sequence, that benefit could be accumulated. For example, binary representation of the number 33 needs only 7 bits (binary: 0100001, plus a sign bit), rather than a full byte. Some practical scenarios are value ranges like GPAs (0 - 5), human heights (0 - a few hundreds cm), or geographic coordinates (-180 to 180).

Take the longitude coordinates on earth for example. Longitude values range from -180.00000° to 180.00000° (5 decimal places). Using the standard IEEE-754 double-precision floats needs a fixed number of 11 exponent bits and 52 mantissa bits, but actually, only 6 exponent bits and 25 mantissa bits are enough to represent upto 5 decimal places. This helps to save 50% of normally used memory per one value. See the [Range test cases](pbb/test/FloatResizerRangeTest.cpp) for details.

1. Support operations

The buffer supports write and read methods that allows input data as byte or integer (32 or 64 bits - long). The important argument is the number of valid bits regarding to the data to write to or read from the buffer. Those are the actual bits that would be actually extracted from the input data to write to the buffer, or bits of data to read from the buffer starting at the current position.

1. Extensible capacity

The buffer is allocated with a small capacity at first and has ability to grow when the data to write exceed the current maximum space.

1. Floating-point compression
1. TODO
    1. Seek
    1. Peek
    1. Capacity maximum reached handling
    1. Add test cases
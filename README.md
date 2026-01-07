Partial Byte Buffer
---
A growable buffer type that supports writing and reading data at bit-level sizes.

1. Why

This buffer supports data at sub-byte level for write and read. For example you have number 33, which needs only 7 bits in binary representation (0100001, with one bit for sign) and you want to store only that 7 bits. This is useful in case the range of your data will never use up a number of full bytes for representation. We can see such scenarios in real life like: GPA (0 -> 5), Human height (0 -> a few hundreds cm), Geographic coordnate (-180 -> 180).

1. Support operations
    1. Buffer Write

    
    1. Buffer Read
1. Extensible capacity

The buffer is allocated with a small capacity at first and has ability to grow when the data to write exceed the current maximum space.

1. Floating-point compression
1. TODO
    1. Seek
    1. Peek
    1. Capacity maximum reached handling
    1. Add test cases
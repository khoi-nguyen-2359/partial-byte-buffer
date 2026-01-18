# Partial Byte Buffer

## 1. Why

This buffer supports writing and reading data at bit-level sizes. This is useful when your data never uses up a number of full bytes in their binary representation. For example, 33 can be represented with only 7 bits (0100001, one bit for sign), rather than a full byte. We can easily find some practical scenarios like value ranges of geographic coordinates (latitude/longitude), or altitude/elevation, temperature, heart rates.

Take the longitude coordinates on Earth for example. The values range from -180.00000° to 180.00000° (5 decimal places). Using the standard IEEE-754 double-precision floats needs a fixed number of 11 exponent bits and 52 mantissa bits. However, 6 exponent bits and 25 mantissa bits are actually enough for values at 5 decimal digit precision. This helps to save around 50% of normally used memory per one value.

| Data Field | Standard IEEE-754 | Float Compression | Savings |
|------------|-------------------|-------------------------|---------|
| Longitude (5 dp) | 64 bits | 32 bits (6 exp + 25 mant + 1 sign) | 50% |
| Latitude (5 dp) | 64 bits | 31 bits (6 exp + 24 mant + 1 sign) | 52% |
| Aircraft Altitude (cm precision) | 32 bits | 27 bits (5 exp + 21 mant + 1 sign) | 16% |
| Temperature (0.01°C) | 32 bits | 23 bits (5 exp + 17 mant + 1 sign) | 28% |

### Outdoor Activity Tracking

One example of where this optimization can be achieved is outdoor tracking file formats (such as TCX, FIT). These files primarily consist of track points, and each track point contains measurements that can be compressed. If the recording frequency is 1 point per second during a 30-minute run, which is a normal rate, there could be numerous track points to save.

## 2. Supported Functions

The buffer supports write and read methods that allow input data as a byte or integer (32 or 64 bits). The key argument is the number of valid bits related to the data to write to or read from the buffer. These are the bits that will be extracted from the input data and written to the buffer, or the bits of data to read from the buffer starting at the current position.

## 3. Expandable Capacity

The buffer can be allocated with an initial capacity and has the ability to grow this size when the data to write exceeds the current maximum space.

There are two capacity growth strategies: **Grow By Double** or **Grow By Half**, which multiply the current size by 2 or 1.5, respectively. This expansion behavior is triggered before an actual write is executed, when the current bits plus the bits to write exceeds the capacity.

## 4. TODOs

| Done | Task |
|---|------|
| ⬜ | Combine two structs of write and read for simplification. |
| ⬜ | Find more data types to add to range tests. |
| ⬜ | Bounded-size capacity behaviour. |
| ⬜ | Full/Empty buffer read/write. |
| ⬜ | Consider unsign floats to save one bit for sign when resizing. |
| ⬜ | Distant memory allocation. |
| ⬜ | Support other operations seek, clear buffer... |

## 5. References
1. Circular Buffer - https://en.wikipedia.org/wiki/Circular_buffer
1. Linux Socket Buffer -  https://dev.to/amrelhusseiny/linux-networking-part-1-kernel-net-stack-180l 
1. Garmin Training Center Database - TCX schema - https://www8.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd

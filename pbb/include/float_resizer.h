#ifndef FLOAT_RESIZER_H
#define FLOAT_RESIZER_H

#include <stdint.h>

/**
 * Union to interpret floating-point binary representation from a double to a 64-bit integer and vice versa.
 */
typedef union qword {
    double double_val;
    uint64_t uint64_val;
} qword;

/**
 * Resize a floating point number represented as uint64_t from source format to destination format.
 * The source and destination formats are defined by the number of exponent bits and mantissa bits.
 * The sign bit is always 1 bit.
 */
uint64_t flr_resize_float_long(
    uint64_t src, 
    int src_exp_bits, int src_mant_bits,
    int dst_exp_bits, int dst_mant_bits
);

/**
 * Resize a floating point number represented as double from source format to destination format.
 * The source and destination formats are defined by the number of exponent bits and mantissa bits.
 * The sign bit is always 1 bit.
 */
uint64_t flr_resize_float_double(
    double src, 
    int src_exp_bits, int src_mant_bits,
    int dst_exp_bits, int dst_mant_bits
);

#endif // FLOAT_RESIZER_H
#ifndef FLOAT_RESIZER_H
#define FLOAT_RESIZER_H

#include <stdint.h>

/**
 * Union to interpret binary representation of different types.
 */
typedef union qword {
    double double_val;
    uint64_t uint64_val;
} qword;

/**
 * Resize a floating point number from source format to destination format.
 * The formats are defined by the number of exponent bits and mantissa bits. 
 * The sign bit is always 1.
 * 
 * @param src The floating point number in its binary representation as uint64_t.
 */
uint64_t flr_resize_float_long(
    uint64_t src, 
    int src_exp_bits, int src_mant_bits,
    int dst_exp_bits, int dst_mant_bits
);

/**
 * A convenience wrapper of flr_resize_float_long that accepts double instead of uint64_t.
 */
uint64_t flr_resize_float_double(
    double src, 
    int src_exp_bits, int src_mant_bits,
    int dst_exp_bits, int dst_mant_bits
);

#endif // FLOAT_RESIZER_H
#ifndef FLOAT_RESIZER_H
#define FLOAT_RESIZER_H

#include <stdint.h>

typedef union qword {
    double double_val;
    uint64_t uint64_val;
} qword;

uint64_t flr_resize_float_long(
    uint64_t src, 
    int src_exp_bits, int src_mant_bits,
    int dst_exp_bits, int dst_mant_bits
);

uint64_t flr_resize_float_double(
    double src, 
    int src_exp_bits, int src_mant_bits,
    int dst_exp_bits, int dst_mant_bits
);

#endif // FLOAT_RESIZER_H
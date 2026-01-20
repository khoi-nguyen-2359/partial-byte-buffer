#include "float_resizer.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

uint64_t flr_resize_float_long(
    uint64_t src, 
    int src_exp_bits, int src_mant_bits,
    int dst_exp_bits, int dst_mant_bits
) {
    uint64_t dst_exp_mask = ((uint64_t)1 << dst_exp_bits) - 1;
    uint64_t dst_bias = dst_exp_mask >> 1;
    uint64_t dst_exponent;
    
    /*
    * Resize exponent
    */
    if (dst_exp_bits == 0) {
        dst_exponent = 0;
    } else if (src_exp_bits == 0) {
        dst_exponent = (1 + dst_bias) & dst_exp_mask;
    } else {
        uint64_t src_exp_mask = ((uint64_t)1 << src_exp_bits) - 1;
        uint64_t src_exponent = (src >> src_mant_bits) & src_exp_mask;
        
        if (src_exponent == 0) {
            dst_exponent = 0;
        } else if (src_exponent == src_exp_mask) {
            dst_exponent = dst_exp_mask;
        } else {
            uint64_t src_bias = src_exp_mask >> 1;
            dst_exponent = (src_exponent - src_bias + dst_bias) & dst_exp_mask;
        }
    }
    
    /*
    * Resize mantissa
    */ 
    uint64_t src_mant_mask = ((uint64_t)1 << src_mant_bits) - 1;
    uint64_t dst_mant_mask = ((uint64_t)1 << dst_mant_bits) - 1;
    int mant_bits_diff = dst_mant_bits - src_mant_bits;
    int mant_shift_left = mant_bits_diff > 0 ? mant_bits_diff : 0;
    int mant_shift_right = - mant_bits_diff > 0 ? - mant_bits_diff : 0;
    uint64_t dst_mant = ((src & src_mant_mask) << mant_shift_left >> mant_shift_right) & dst_mant_mask;
    
    /*
    * Combine sign, exponent, and mantissa
    */ 
    uint64_t dst_sign = (src >> (src_exp_bits + src_mant_bits)) & 1;
    return (((dst_sign << dst_exp_bits) | dst_exponent) << dst_mant_bits) | dst_mant;
}

uint64_t flr_resize_float_double(
    double src, 
    int src_exp_bits, int src_mant_bits,
    int dst_exp_bits, int dst_mant_bits
) {
    qword wq;
    wq.double_val = src;
    return flr_resize_float_long(wq.uint64_val, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
}

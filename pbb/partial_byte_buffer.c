#include "partial_byte_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(val, min, max) ( (val) < (min) ? (min) : ( (val) > (max) ? (max) : (val) ) )

static const uint8_t BITSIZEOF_INT = sizeof(int) << 3;
static const uint8_t BITSIZEOF_LONG = sizeof(long) << 3;
static const uint8_t BITSIZEOF_INT64 = sizeof(int64_t) << 3;

static const int CAPACITY_DOUBLE = 0;
static const int CAPACITY_HALF = 1;

#ifndef CAPACITY_GROWTH_MODE
#define CAPACITY_GROWTH_MODE CAPACITY_DOUBLE
#endif

/**
 * Extracted function to write data into the current byte of partial_byte_buffer.
 * This function is called multiple times to write a number of data bits at a time, until all bits are written.
 * @param data The data to write. Use 64-bit to accommodate supported types.
 * @param data_bits Number of data bits remaining, will be updated after each succeeded write.
 */
static void write_byte(partial_byte_buffer* pbb, uint64_t data, uint8_t* data_bits);

/**
 * Extracted function to read a byte with specified bit length from the buffer into an accumulator.
 * This function is called multiple times by a higher-level read method until all bits are read.
 * @param acc Accumulator to store the read value. Use 64-bit to accommodate supported types.
 * @param read_bits Number of bits remaining to read; will be updated after each successful read.
 */
static uint8_t read_byte(partial_byte_buffer* pbbr, uint64_t* acc, uint8_t* read_bits);

/**
 * Return a sensible minimum number of bytes for reading [bits] bits from the current position.
 * This is used to check if there is enough data in the buffer before reading.
 */
static size_t required_length(const partial_byte_buffer* pbbr, uint8_t bits);

/**
 * Find a right allocation size to cover [n] bytes of buffer.
 * @param n Number of bytes requested.
 */
static size_t next_capacity(size_t n);

/**
 * Ensure a partial_byte_buffer has enough capacity to write [bits] more bits 
 * by reallocating its internal buffer if necessary.
 */
static void ensure_capacity(partial_byte_buffer* pbb, uint8_t bits);

/**
 * Extend the sign bit of a 64-bit value from [bits] bits to a full 64-bit integer.
 */
static void extend_sign(uint64_t* value, uint8_t bits);

partial_byte_buffer* pbb_create(int initial_capacity) {
    if (initial_capacity <= 0) return NULL;
    
    partial_byte_buffer* pbb = (partial_byte_buffer*)malloc(sizeof(partial_byte_buffer));
    if (pbb == NULL) return NULL;
    
    /**
     * calloc initializes memory to zero, which is neccessary for bitwise operations.
     */
    pbb->buffer = (uint8_t*)calloc(initial_capacity, 1);
    if (pbb->buffer == NULL) {
        free(pbb);
        return NULL;
    }

    pbb->capacity = initial_capacity;
    pbb->write_pos = 0;
    pbb->read_pos = 0;

    return pbb;
}

partial_byte_buffer* pbb_from_array(const uint8_t* array, size_t size) {
    if (array == NULL || size == 0) return NULL;
    
    partial_byte_buffer* pbb = (partial_byte_buffer*)malloc(sizeof(partial_byte_buffer));
    if (pbb == NULL) return NULL;
    
    pbb->buffer = (uint8_t*)malloc(size);
    if (pbb->buffer == NULL) {
        free(pbb);
        return NULL;
    }
    
    memcpy(pbb->buffer, array, size);
    pbb->capacity = size;
    pbb->write_pos = size * 8;
    pbb->read_pos = 0;
    
    return pbb;
}

void pbb_destroy(partial_byte_buffer** pbb) {
    if (*pbb != NULL && (*pbb)->buffer != NULL) free((*pbb)->buffer);
    if (pbb != NULL) free(*pbb);
    *pbb = NULL;
}

size_t pbb_get_length(const partial_byte_buffer* pbb) {
    if (pbb == NULL) return 0;
    return (pbb->write_pos + 7) >> 3;
}

void pbb_write_byte(partial_byte_buffer* pbb, int8_t byte, uint8_t bits) {
    if (pbb == NULL || bits <= 0 || bits > 8) return;

    ensure_capacity(pbb, bits);

    uint8_t remaining_bits = bits;
    while (remaining_bits > 0) {
        write_byte(pbb, byte, &remaining_bits);
    }
}

void pbb_write_int(partial_byte_buffer* pbb, int value, uint8_t bits) {
    if (pbb == NULL || bits <= 0 || bits > BITSIZEOF_INT) return;

    ensure_capacity(pbb, bits);
    
    uint8_t remaining_bits = bits;
    while (remaining_bits > 0) {
        write_byte(pbb, value, &remaining_bits);
    }
}

int8_t pbb_read_byte(partial_byte_buffer* pbbr, uint8_t bits) {
    if (pbbr == NULL || bits <= 0 || bits > 8) return 0;
    if (required_length(pbbr, bits) > pbb_get_length(pbbr)) return 0;

    uint64_t result = 0;
    uint8_t remaining_bit_len = bits;
    
    while (remaining_bit_len > 0) {
        read_byte(pbbr, &result, &remaining_bit_len);
    }

    if (bits < 8) {
        extend_sign(&result, bits);
    }
    
    return (int8_t)result;
}

int pbb_read_int(partial_byte_buffer* pbbr, uint8_t bits) {
    if (pbbr == NULL || bits <= 0 || bits > BITSIZEOF_INT) return 0;
    if (required_length(pbbr, bits) > pbb_get_length(pbbr)) return 0;

    uint64_t result = 0;
    uint8_t remaining_bit_len = bits;
    
    while (remaining_bit_len > 0) {
        read_byte(pbbr, &result, &remaining_bit_len);
    }

    // Sign extension for negative values
    if (bits < BITSIZEOF_INT) {
        extend_sign(&result, bits);
    }
    
    return (int) result;
}

void pbb_write_int64(partial_byte_buffer* pbb, int64_t value, uint8_t bits) {
    if (pbb == NULL || bits <= 0 || bits > BITSIZEOF_INT64) return;

    ensure_capacity(pbb, bits);
    
    uint8_t remaining_bits = bits;
    while (remaining_bits > 0) {
        write_byte(pbb, value, &remaining_bits);
    }
}

int64_t pbb_read_int64(partial_byte_buffer* pbbr, uint8_t bits) {
    if (pbbr == NULL || bits <= 0 || bits > BITSIZEOF_INT64) return 0;
    if (required_length(pbbr, bits) > pbb_get_length(pbbr)) return 0;

    uint64_t result = 0;
    uint8_t remaining_bit_len = bits;
    
    while (remaining_bit_len > 0) {
        read_byte(pbbr, &result, &remaining_bit_len);
    }

    // Sign extension for negative values
    if (bits < BITSIZEOF_INT64) {
        extend_sign(&result, bits);
    }
    
    return (int64_t) result;
}

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

static uint8_t read_byte(partial_byte_buffer* pbbr, uint64_t* acc, uint8_t* read_bits) {
    uint8_t available_bits = 8 - (pbbr->read_pos & 7);
    uint8_t bits_to_read = MIN(available_bits, *read_bits);
    
    size_t byte_pos = pbbr->read_pos >> 3;
    uint8_t bit_pos = pbbr->read_pos & 7;
    // Extract bits from the buffer at current position
    uint8_t read = (uint8_t)(pbbr->buffer[byte_pos] << bit_pos) >> (8 - bits_to_read);
    *acc = (*acc << bits_to_read) | read;

    // Update position
    pbbr->read_pos += bits_to_read;
    *read_bits -= bits_to_read;
    
    return read;
}

static void write_byte(partial_byte_buffer* pbb, uint64_t data, uint8_t* data_bits) {
    /** 
     * Concatenate data bits at the write cursor position.
     */
    size_t byte_pos = pbb->write_pos >> 3;
    uint8_t available_bits = 8 - (pbb->write_pos & 7);
    uint8_t write_bits = MIN(available_bits, *data_bits);
    pbb->buffer[byte_pos] |= data << (64 - *data_bits) >> (64 - write_bits) << (available_bits - write_bits);

    // Update the write cursor
    pbb->write_pos += write_bits;

    *data_bits -= write_bits;
}

static size_t next_capacity(size_t n) {
    switch (CAPACITY_GROWTH_MODE)
    {
    case CAPACITY_DOUBLE:
        return n << 1;
    case CAPACITY_HALF:
        return n + (n >> 1);
    default:
        break;
    }
}

static void ensure_capacity(partial_byte_buffer* pbb, uint8_t bits) {
    size_t required_bytes = (pbb->write_pos + bits + 7) >> 3;
    if (required_bytes <= pbb->capacity)
        return;

    /**
     * Calculate the new capacity for the buffer.
     * If the new capacity based on current capacity is still not enough,
     * use the required bytes to calculate a larger capacity. 
     */
    size_t capacity = next_capacity(pbb->capacity);
    if (capacity < required_bytes) {
        capacity = next_capacity(required_bytes);
    }

    uint8_t* new_buffer = (uint8_t*)realloc(pbb->buffer, capacity);
    if (new_buffer == NULL) return;

    // Initialize the newly allocated memory to zero
    memset(new_buffer + pbb->capacity, 0, capacity - pbb->capacity);
    pbb->buffer = new_buffer;
    pbb->capacity = capacity;
}

static void extend_sign(uint64_t* value, uint8_t bits) {
    if (bits == 0 || bits >= 64) return;
    
    if (*value & ((uint64_t)1 << (bits - 1))) {
        *value |= (uint64_t)-1 << bits;
    }
}

static size_t required_length(const partial_byte_buffer* pbbr, uint8_t bit_len) {
    return (pbbr->read_pos + bit_len + 7) >> 3;
}

#ifndef PARTIAL_BYTE_BUFFER_H
#define PARTIAL_BYTE_BUFFER_H

#include <stdint.h>
#include <stddef.h>

/**
 * Union to interpret binary representation of different types.
 */
typedef union qword {
    double double_val;
    float float_val;
    uint64_t uint64_val;
    uint32_t uint32_val;
    int32_t int32_val;
} qword;

typedef struct partial_byte_buffer {
    /**
     * Array of bytes storing the buffer data.
     */
    uint8_t* buffer;

    /**
     * Number of bytes allocated for the buffer array.
     */
    size_t capacity;

    /**
     * Bit position for the next write operation.
     */
    size_t write_pos;

    /**
     * Bit position for the next read operation.
     */
    size_t read_pos;
} partial_byte_buffer;

/**
 * Create a partial_byte_buffer with the specified initial capacity in bytes.
 * The actual allocated capacity will be the smallest power of 2 which is greater than or equal to [initial_capacity].
 * Returns NULL for invalid initial_capacity or if memory allocation fails.
 */
partial_byte_buffer* pbb_create(int initial_capacity);

/**
 * Create a partial_byte_buffer from an existing byte array with fixed size.
 * The data is copied into the newly created buffer.
 * Returns NULL for invalid parameters or if memory allocation fails.
 */
partial_byte_buffer* pbb_from_array(const uint8_t* array, size_t size);

/**
 * Destroy a partial_byte_buffer and free its resources.
 * Sets the pointer to NULL after destruction.
 */
void pbb_destroy(partial_byte_buffer** pbb);

/**
 * Get the number of bytes that have been written to a partial_byte_buffer.
 */
size_t pbb_get_length(const partial_byte_buffer* pbb);

/**
 * Write a byte having a length of [bits] (1-8) to the buffer.
 */
void pbb_write_byte(partial_byte_buffer* pbb, int8_t byte, uint8_t bits);

/**
 * Write an integer having a length of [bits] (1-32) to the buffer.
 */
void pbb_write_int(partial_byte_buffer* pbb, int value, uint8_t bits);

/**
 * Read a signed byte having a length of [bits] (1-8) from the buffer.
 */
int8_t pbb_read_byte(partial_byte_buffer* pbbr, uint8_t bits);

/**
 * Read a signed integer having a length of [bits] (1-32) from the buffer.
 */
int pbb_read_int(partial_byte_buffer* pbbr, uint8_t bits);

/**
 * Write a 32-bit integer having a length of [bits] (1-32) to the buffer.
 */
void pbb_write_int32(partial_byte_buffer* pbb, int32_t value, uint8_t bits);

/**
 * Read a signed 32-bit integer having a length of [bits] (1-32) from the buffer.
 */
int32_t pbb_read_int32(partial_byte_buffer* pbbr, uint8_t bits);

/**
 * Write a single-precision float (32 bits) to the buffer.
 */
void pbb_write_float(partial_byte_buffer* pbb, float value);

/**
 * Read a single-precision float (32 bits) from the buffer.
 */
float pbb_read_float(partial_byte_buffer* pbbr);

/**
 * Write a 64-bit integer having a length of [bits] (1-64) to the buffer.
 */
void pbb_write_int64(partial_byte_buffer* pbb, int64_t value, uint8_t bits);

/**
 * Read a signed 64-bit integer having a length of [bits] (1-64) from the buffer.
 */
int64_t pbb_read_int64(partial_byte_buffer* pbbr, uint8_t bits);

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

#endif // PARTIAL_BYTE_BUFFER_H

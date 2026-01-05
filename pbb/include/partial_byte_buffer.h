#ifndef PARTIAL_BYTE_BUFFER_H
#define PARTIAL_BYTE_BUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef struct partial_byte_buffer {
    uint8_t* buffer;
    size_t capacity;
    size_t byte_pos;
    uint8_t bit_pos;
} partial_byte_buffer;

/**
 * Create a partial_byte_buffer with the specified initial capacity in bytes.
 * The actual allocated capacity will be the smallest power of 2 which is greater than or equal to [initial_capacity].
 * Returns NULL for invalid initial_capacity or if memory allocation fails.
 */
partial_byte_buffer* pbb_create(int initial_capacity);

/**
 * Destroy a partial_byte_buffer and free its resources.
 * Sets the pointer to NULL after destruction.
 */
void pbb_destroy(partial_byte_buffer** pbb);

/**
 * Get the number of bytes have been written in a partial_byte_buffer.
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
 * Clone the partial_byte_buffer's buffer to a newly allocated byte array.
 * The size of the array is returned via [len].
 * Returns NULL if the buffer is empty or if [pbb] is NULL.
 * The caller is responsible for freeing the returned array.
 */
uint8_t* pbb_to_byte_array(const partial_byte_buffer* pbb, size_t* len);

/**
 * Get the internal buffer array of the partial_byte_buffer.
 * The size of the valid data in the buffer is returned via [len].
 * Returns NULL if [pbb] is NULL.
 */
uint8_t* pbb_get_buffer_array(const partial_byte_buffer* pbb, size_t* len);

#endif // PARTIAL_BYTE_BUFFER_H

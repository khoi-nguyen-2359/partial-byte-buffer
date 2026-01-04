#ifndef PARTIAL_BYTE_BUFFER_H
#define PARTIAL_BYTE_BUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef struct PartialByteBuffer {
    uint8_t* buffer;
    size_t capacity;
    size_t byte_pos;
    uint8_t bit_pos;
} PartialByteBuffer;

/**
 * Create a PartialByteBuffer with the specified initial capacity in bytes.
 * The actual allocated capacity will be the smallest power of 2 which is greater than or equal to [initial_capacity].
 * Returns NULL for invalid initial_capacity or if memory allocation fails.
 */
PartialByteBuffer* pbb_create(int initial_capacity);

/**
 * Destroy a PartialByteBuffer and free its resources.
 * Sets the pointer to NULL after destruction.
 */
void pbb_destroy(PartialByteBuffer** pbb);

/**
 * Get the number of bytes have been written in a PartialByteBuffer.
 */
size_t pbb_get_length(const PartialByteBuffer* pbb);

/**
 * Write a byte having a length of [bits] (1-8) to the buffer.
 */
void pbb_write_byte(PartialByteBuffer* pbb, int8_t byte, uint8_t bits);

/**
 * Write an integer having a length of [bits] (1-32) to the buffer.
 */
void pbb_write_int(PartialByteBuffer* pbb, int value, uint8_t bits);

/**
 * Clone the PartialByteBuffer's buffer to a newly allocated byte array.
 * The size of the array is returned via [len].
 * Returns NULL if the buffer is empty or if [pbb] is NULL.
 * The caller is responsible for freeing the returned array.
 */
uint8_t* pbb_to_byte_array(const PartialByteBuffer* pbb, size_t* len);

/**
 * Get the internal buffer array of the PartialByteBuffer.
 * The size of the valid data in the buffer is returned via [len].
 * Returns NULL if [pbb] is NULL.
 */
uint8_t* pbb_get_buffer_array(const PartialByteBuffer* pbb, size_t* len);

#endif // PARTIAL_BYTE_BUFFER_H

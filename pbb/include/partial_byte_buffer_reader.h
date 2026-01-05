#ifndef PARTIAL_BYTE_BUFFER_READER_H
#define PARTIAL_BYTE_BUFFER_READER_H

#include <stdint.h>
#include <stddef.h>

typedef struct partial_byte_buffer_reader {
    uint8_t* buffer;
    size_t length;
    size_t byte_pos;
    uint8_t bit_pos;
} partial_byte_buffer_reader;

/**
 * Create a PartialByteBufferReader from an existing byte buffer and length.
 * Returns NULL for invalid input or if memory allocation fails.
 */
partial_byte_buffer_reader* pbbr_create(const uint8_t* buffer, size_t length);

/**
 * Destroy a PartialByteBufferReader and free its resources.
 * Sets the pointer to NULL after destruction.
 */
void pbbr_destroy(partial_byte_buffer_reader** pbbr);

/**
 * Read a signed byte having a length of [bits] (1-8) from the buffer.
 */
int8_t pbbr_read_byte(partial_byte_buffer_reader* pbbr, uint8_t bits);

/**
 * Read a signed integer having a length of [bits] (1-32) from the buffer.
 */
int pbbr_read_int(partial_byte_buffer_reader* pbbr, uint8_t bits);

#endif // PARTIAL_BYTE_BUFFER_READER_H
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

static const int CAPACITY_DOUBLE = 0;
static const int CAPACITY_HALF = 1;

#ifndef CAPACITY_GROWTH_MODE
#define CAPACITY_GROWTH_MODE CAPACITY_DOUBLE
#endif

/**
 * Extracted function to write data into the current byte of partial_byte_buffer.
 * This function is called multiple times to write a number of data bits at a time, until all bits are written.
 * @param data The data to write.
 * @param data_bits Number of data bits remaining, will be updated after each succeeded write.
 * @param available_bits Number of bits available for write in the current byte of the buffer.
 */
static void write_byte(partial_byte_buffer* pbb, uint32_t data, uint8_t* data_bits, uint8_t available_bits);

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
    pbb->bit_pos = 0;
    pbb->byte_pos = 0;

    return pbb;
}

void pbb_destroy(partial_byte_buffer** pbb) {
    if ((*pbb)->buffer != NULL) free((*pbb)->buffer);
    if (pbb != NULL) free(*pbb);
    *pbb = NULL;
}

size_t pbb_get_length(const partial_byte_buffer* pbb) {
    if (pbb == NULL) return 0;
    return pbb->byte_pos + ((pbb->bit_pos + 7) >> 3);
}

void pbb_write_byte(partial_byte_buffer* pbb, int8_t byte, uint8_t bits) {
    if (pbb == NULL || bits <= 0 || bits > 8) return;

    ensure_capacity(pbb, bits);

    uint8_t remaining_bits = bits;
    while (remaining_bits > 0) {
        write_byte(pbb, byte, &remaining_bits, 8 - pbb->bit_pos);
    }
}

void pbb_write_int(partial_byte_buffer* pbb, int value, uint8_t bits) {
    if (pbb == NULL || bits <= 0 || bits > BITSIZEOF_INT) return;

    ensure_capacity(pbb, bits);
    
    uint8_t remaining_bits = bits;
    while (remaining_bits > 0) {
        write_byte(pbb, value, &remaining_bits, 8 - pbb->bit_pos);
    }
}

uint8_t* pbb_to_byte_array(const partial_byte_buffer* pbb, size_t* len) {
    size_t byte_count = pbb_get_length(pbb);
    if (len != NULL) {
        *len = byte_count;
    }

    if (pbb == NULL) return NULL;
    if (byte_count == 0) return NULL;

    uint8_t* result = (uint8_t*)malloc(byte_count);
    if (result == NULL) return NULL;
    
    memcpy(result, pbb->buffer, byte_count);
    return result;
}

uint8_t* pbb_get_buffer_array(const partial_byte_buffer* pbb, size_t* len) {
    if (len != NULL) {
        *len = pbb_get_length(pbb);
    }
    if (pbb == NULL) return NULL;
    return pbb->buffer;
}

static void write_byte(partial_byte_buffer* pbb, uint32_t data, uint8_t* data_bits, uint8_t available_bits) {
    /** 
     * Concatenate data bits at the bit cursor of the current byte
     */
    uint8_t write_bits = MIN(available_bits, *data_bits);
    pbb->buffer[pbb->byte_pos] |= data << (32 - *data_bits) >> (32 - write_bits) << (8 - pbb->bit_pos - write_bits);

    /**
     * Update positions of byte and bit cursors
     */
    uint8_t next_bit_pos = pbb->bit_pos + write_bits;
    pbb->byte_pos += next_bit_pos >> 3;
    pbb->bit_pos = next_bit_pos & 7;

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
    size_t required_bytes = pbb->byte_pos + ((pbb->bit_pos + bits + 7) >> 3);
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

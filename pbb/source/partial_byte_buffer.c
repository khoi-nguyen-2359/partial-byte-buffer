#include "partial_byte_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define MIN_INIT_LEN 2
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(val, min, max) ( (val) < (min) ? (min) : ( (val) > (max) ? (max) : (val) ) )

static const uint8_t BITSIZEOF_INT = sizeof(int) << 3;
static const uint8_t BITSIZEOF_LONG = sizeof(long) << 3;

/**
 * Extracted function to write bits of [data] into the current byte of a PartialByteBuffer.
 * [data_bits] will be updated to reflect the remaining bits of the data to write.
 */
static void write_byte(PartialByteBuffer* pbb, uint32_t data, uint8_t* data_bits, uint8_t available_bits);

/**
 * Find an allocation size to cover [n] bytes of buffer.
 */
static size_t next_capacity(size_t n);

/**
 * Ensure that the PartialByteBuffer has enough capacity to write [bits] more bits.
 * If not, the buffer is reallocated to a larger size.
 */
static void ensure_capacity(PartialByteBuffer* pbb, uint8_t bits);

/**
 * Return the number with the highest one bit in an integer value on.
 * Returns 0 if the value is 0.
 * For example, highest_one_bit(18) returns 16.
 */
static unsigned int highest_one_bit(int value);

PartialByteBuffer* pbb_create(int initial_capacity) {
    if (initial_capacity <= 0) return NULL;
    
    PartialByteBuffer* pbb = (PartialByteBuffer*)malloc(sizeof(PartialByteBuffer));
    if (pbb != NULL) {
        size_t capacity = highest_one_bit(initial_capacity);
        if (capacity < initial_capacity) {
            capacity = next_capacity(capacity);
        }
        pbb->buffer = (uint8_t*)calloc(capacity, sizeof(uint8_t));
        pbb->capacity = capacity;
        pbb->bit_pos = 0;
        pbb->byte_pos = 0;
    }
    return pbb;
}

void pbb_destroy(PartialByteBuffer** pbb) {
    if (pbb != NULL && *pbb != NULL) {
        free((*pbb)->buffer);
        free(*pbb);
        *pbb = NULL;
    }
}

size_t pbb_get_length(const PartialByteBuffer* pbb) {
    if (pbb == NULL) return 0;
    return pbb->byte_pos + ((pbb->bit_pos + 7) >> 3);
}

void pbb_write_byte(PartialByteBuffer* pbb, int8_t byte, uint8_t bits) {
    if (pbb == NULL || bits <= 0 || bits > 8) return;

    ensure_capacity(pbb, bits);

    uint8_t remaining_bits = bits;
    write_byte(pbb, byte, &remaining_bits, 8 - pbb->bit_pos);
    write_byte(pbb, byte, &remaining_bits, 8);
}

void pbb_write_int(PartialByteBuffer* pbb, int value, uint8_t bits) {
    if (pbb == NULL || bits <= 0 || bits > BITSIZEOF_INT) return;
    ensure_capacity(pbb, bits);
    
    uint8_t remaining_bits = bits;
    write_byte(pbb, value, &remaining_bits, 8 - pbb->bit_pos);
    for (int i = 0; i < 3; ++i) {
        write_byte(pbb, value, &remaining_bits, 8);
    }
}

uint8_t* pbb_to_byte_array(const PartialByteBuffer* pbb, size_t* len) {
    size_t byte_count = pbb_get_length(pbb);
    if (len != NULL) {
        *len = byte_count;
    }

    if (pbb == NULL) return NULL;
    if (byte_count == 0) return NULL;

    uint8_t* result = (uint8_t*)malloc(byte_count);
    if (result != NULL) {
        memcpy(result, pbb->buffer, byte_count);
    }
    return result;
}

uint8_t* pbb_get_buffer_array(const PartialByteBuffer* pbb, size_t* len) {
    if (len != NULL) {
        *len = pbb_get_length(pbb);
    }
    if (pbb == NULL) return NULL;
    return pbb->buffer;
}

static void write_byte(PartialByteBuffer* pbb, uint32_t data, uint8_t* data_bits, uint8_t available_bits) {
    if (*data_bits <= 0) return;

    uint8_t write_bits = MIN(available_bits, *data_bits);
    pbb->buffer[pbb->byte_pos] |= data << (32 - *data_bits) >> (32 - write_bits) << (8 - pbb->bit_pos - write_bits);
    uint8_t next_bit_pos = pbb->bit_pos + write_bits;
    pbb->byte_pos += next_bit_pos >> 3;
    pbb->bit_pos = next_bit_pos & 7;
    *data_bits -= write_bits;
}

static unsigned int highest_one_bit(int value) {
    if (value == 0) return 0;

    unsigned int position = 0;
    
    if (value >= 0x10000) { position += 16; value >>= 16; }
    if (value >= 0x100)   { position += 8;  value >>= 8; }
    if (value >= 0x10)    { position += 4;  value >>= 4; }
    if (value >= 0x4)     { position += 2;  value >>= 2; }
    if (value >= 0x2)     { position += 1; }

    return 1U << position;
}

static size_t next_capacity(size_t n) {
    size_t next_power2 = n << 1;
    if (next_power2 <= n) {
        return n | n >> 1;
    }
    return next_power2;
}

static void ensure_capacity(PartialByteBuffer* pbb, uint8_t bits) {
    size_t required_bytes = pbb->byte_pos + ((pbb->bit_pos + bits + 7) >> 3);
    if (required_bytes <= pbb->capacity)
        return;

    size_t capacity = next_capacity(pbb->capacity);
    if (capacity < required_bytes) {
        capacity = next_capacity(highest_one_bit(required_bytes));
    }
    uint8_t* new_buffer = (uint8_t*)realloc(pbb->buffer, capacity);
    if (new_buffer != NULL) {   
        memset(new_buffer + pbb->capacity, 0, capacity - pbb->capacity);
        pbb->buffer = new_buffer;
        pbb->capacity = capacity;
    }
}

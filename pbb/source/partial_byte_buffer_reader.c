#include "partial_byte_buffer_reader.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const uint8_t BITSIZEOF_INT = sizeof(int) << 3;

/**
 * Extracted function to read a byte with specified bit length from the buffer.
 * This function is called multiple times in a higher read method until all bits are read.
 * @param acc Accumulator to store the read value.
 * @param read_bits Number of bits remaining to read, will be updated after each succeeded read.
 * @param available_bits Number of bits available to read in the current byte of the buffer.
 */
static uint8_t read_byte(partial_byte_buffer_reader* pbbr, uint32_t* acc, uint8_t* read_bits, uint8_t available_bits);

/**
 * Calculate the number of bytes enough for a read of [bits] bits from the current position.
 * This is used to check if there is enough data in the buffer before reading.
 */
static size_t required_length(const partial_byte_buffer_reader* pbbr, uint8_t bits);

/**
 * Extend the sign bit of a value read with [bits] bits to a full integer.
 */
static void extend_sign(uint32_t* value, uint8_t bits);

partial_byte_buffer_reader* pbbr_create(const uint8_t* buffer, size_t length) {
    if (buffer == NULL || length == 0) return NULL;

    partial_byte_buffer_reader* pbbr = (partial_byte_buffer_reader*)malloc(sizeof(partial_byte_buffer_reader));
    if (pbbr != NULL) {
        uint8_t* buffer_copy = (uint8_t*)malloc(length);
        if (buffer_copy != NULL) {
            memcpy(buffer_copy, buffer, length);
            pbbr->buffer = buffer_copy;
            pbbr->length = length;
            pbbr->byte_pos = 0;
            pbbr->bit_pos = 0;
        } else {
            free(pbbr);
            pbbr = NULL;
        }
    }
    return pbbr;
}

void pbbr_destroy(partial_byte_buffer_reader** pbbr) {
    if (pbbr != NULL && *pbbr != NULL) {
        free((*pbbr)->buffer);
        free(*pbbr);
        *pbbr = NULL;
    }
}

int8_t pbbr_read_byte(partial_byte_buffer_reader* pbbr, uint8_t bits) {
    if (pbbr == NULL || bits <= 0 || bits > 8) return 0;
    if (required_length(pbbr, bits) > pbbr->length) return 0;

    uint32_t result = 0;
    uint8_t remaining_bit_len = bits;
    
    while (remaining_bit_len > 0) {
        read_byte(pbbr, &result, &remaining_bit_len, 8 - pbbr->bit_pos);
    }

    if (bits < 8) {
        extend_sign(&result, bits);
    }
    
    return (int8_t)result;
}

int pbbr_read_int(partial_byte_buffer_reader* pbbr, uint8_t bits) {
    if (pbbr == NULL || bits <= 0 || bits > BITSIZEOF_INT) return 0;
    if (required_length(pbbr, bits) > pbbr->length) return 0;

    uint32_t result = 0;
    uint8_t remaining_bit_len = bits;
    
    while (remaining_bit_len > 0)
        read_byte(pbbr, &result, &remaining_bit_len, 8 - pbbr->bit_pos);

    // Sign extension for negative values
    if (bits < BITSIZEOF_INT) {
        extend_sign(&result, bits);
    }
    
    return (int) result;
}

static uint8_t read_byte(partial_byte_buffer_reader* pbbr, uint32_t* acc, uint8_t* read_bits, uint8_t available_bits) {
    uint8_t bits_to_read = MIN(available_bits, *read_bits);
    
    // Extract bits from the buffer at current position
    uint8_t read = (uint8_t)(pbbr->buffer[pbbr->byte_pos] << pbbr->bit_pos) >> (8 - bits_to_read);
    // printf("  Extracted bits: 0x%X\n", read);
    *acc = (*acc << bits_to_read) | read;
    // Update position
    uint8_t next_bit_pos = pbbr->bit_pos + bits_to_read;
    pbbr->byte_pos += next_bit_pos >> 3;
    pbbr->bit_pos = next_bit_pos & 7;
    *read_bits -= bits_to_read;
    
    return read;
}

static size_t required_length(const partial_byte_buffer_reader* pbbr, uint8_t bit_len) {
    return pbbr->byte_pos + ((pbbr->bit_pos + bit_len + 7) >> 3);
}

static void extend_sign(uint32_t* value, uint8_t bits) {
    if (*value & (1 << (bits - 1))) {
        *value |= -1 << bits;
    }
}
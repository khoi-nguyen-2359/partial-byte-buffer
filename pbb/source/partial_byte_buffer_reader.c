#include "partial_byte_buffer_reader.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const uint8_t BITSIZEOF_UINT = sizeof(unsigned int) << 3;
static const uint8_t BITSIZEOF_INT = sizeof(int) << 3;

static uint8_t read_byte(PartialByteBufferReader* pbbr, uint32_t* acc, uint8_t* read_bit_len, uint8_t available_bit_len);
static size_t required_length(const PartialByteBufferReader* pbbr, uint8_t bit_len);
static void extend_sign(uint32_t* value, uint8_t bit_len);

PartialByteBufferReader* pbbr_create(const uint8_t* buffer, size_t length) {
    if (buffer == NULL || length == 0) return NULL;

    PartialByteBufferReader* pbbr = (PartialByteBufferReader*)malloc(sizeof(PartialByteBufferReader));
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

void pbbr_destroy(PartialByteBufferReader** pbbr) {
    if (pbbr != NULL && *pbbr != NULL) {
        free((*pbbr)->buffer);
        free(*pbbr);
        *pbbr = NULL;
    }
}

int8_t pbbr_read_byte(PartialByteBufferReader* pbbr, uint8_t bit_len) {
    if (pbbr == NULL || bit_len <= 0 || bit_len > 8) return 0;
    if (required_length(pbbr, bit_len) > pbbr->length) return 0;

    uint32_t result = 0;
    uint8_t remaining_bit_len = bit_len;
    
    read_byte(pbbr, &result, &remaining_bit_len, 8 - pbbr->bit_pos);
    read_byte(pbbr, &result, &remaining_bit_len, 8);

    if (bit_len < 8) {
        extend_sign(&result, bit_len);
    }
    
    return (int8_t)result;
}

int pbbr_read_int(PartialByteBufferReader* pbbr, uint8_t bit_len) {
    if (pbbr == NULL || bit_len <= 0 || bit_len > BITSIZEOF_INT) return 0;
    if (required_length(pbbr, bit_len) > pbbr->length) return 0;

    uint32_t result = 0;
    uint8_t remaining_bit_len = bit_len;
    
    read_byte(pbbr, &result, &remaining_bit_len, 8 - pbbr->bit_pos);
    while (remaining_bit_len > 0) {
        read_byte(pbbr, &result, &remaining_bit_len, 8);
    }

    // Sign extension for negative values
    if (bit_len < BITSIZEOF_INT) {
        extend_sign(&result, bit_len);
    }
    
    return (int) result;
}

static uint8_t read_byte(PartialByteBufferReader* pbbr, uint32_t* acc, uint8_t* read_bit_len, uint8_t available_bit_len) {
    if (*read_bit_len <= 0) return 0;
    // printf("Reading %u bits (available: %u bits) at byte_pos: %zu, bit_pos: %u\n", *read_bit_len, available_bit_len, pbbr->byte_pos, pbbr->bit_pos);
    
    uint8_t bits_to_read = MIN(available_bit_len, *read_bit_len);
    
    // Extract bits from the buffer at current position
    uint8_t read = (uint8_t)(pbbr->buffer[pbbr->byte_pos] << pbbr->bit_pos) >> (8 - bits_to_read);
    // printf("  Extracted bits: 0x%X\n", read);
    *acc = (*acc << bits_to_read) | read;
    // Update position
    uint8_t next_bit_pos = pbbr->bit_pos + bits_to_read;
    pbbr->byte_pos += next_bit_pos >> 3;
    pbbr->bit_pos = next_bit_pos & 7;
    *read_bit_len -= bits_to_read;
    
    return read;
}

static size_t required_length(const PartialByteBufferReader* pbbr, uint8_t bit_len) {
    return pbbr->byte_pos + ((pbbr->bit_pos + bit_len + 7) >> 3);
}

static void extend_sign(uint32_t* value, uint8_t bit_len) {
    if (*value & (1 << (bit_len - 1))) {
        *value |= -1 << bit_len;
    }
}
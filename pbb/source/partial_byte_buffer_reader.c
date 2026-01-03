#include "partial_byte_buffer_reader.h"

#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const uint8_t BITSIZEOF_UINT = sizeof(unsigned int) << 3;
static const uint8_t BITSIZEOF_INT = sizeof(int) << 3;

static uint8_t read_byte(PartialByteBufferReader* pbbr, uint8_t* read_bit_len, uint8_t available_bit_len);

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
    if ((pbbr->byte_pos + (pbbr->bit_pos + bit_len)) >> 3 >= pbbr->length) return 0;

    unsigned int result = 0;
    uint8_t remaining_bit_len = bit_len;
    
    result |= read_byte(pbbr, &remaining_bit_len, 8 - pbbr->bit_pos);
    result |= read_byte(pbbr, &remaining_bit_len, 8);
    
    return (int8_t)result;
}

int pbbr_read_int(PartialByteBufferReader* pbbr, uint8_t bit_len) {
    if (pbbr == NULL || bit_len <= 0 || bit_len > BITSIZEOF_INT) return 0;
    if (pbbr->byte_pos >= pbbr->length) return 0;

    unsigned int result = 0;
    uint8_t remaining_bit_len = bit_len;
    
    result |= read_byte(pbbr, &remaining_bit_len, 8 - pbbr->bit_pos);
    for (int i = 0; i < 3; ++i) {
        result |= read_byte(pbbr, &remaining_bit_len, 8);
    }
    
    // Sign extend if necessary (if the most significant bit of the read data is 1)
    if (bit_len < BITSIZEOF_INT && (result & (1U << (bit_len - 1)))) {
        result |= (0xFFFFFFFF << bit_len);
    }
    
    return (int)result;
}

static uint8_t read_byte(PartialByteBufferReader* pbbr, uint8_t* read_bit_len, uint8_t available_bit_len) {
    if (*read_bit_len <= 0) return 0;
    if (pbbr->byte_pos >= pbbr->length) return 0;
    
    uint8_t bits_to_read = MIN(available_bit_len, *read_bit_len);
    
    // Extract bits from the buffer at current position
    uint8_t result = (uint8_t)(pbbr->buffer[pbbr->byte_pos] << pbbr->bit_pos) >> (8 - bits_to_read) << (MIN(*read_bit_len, 8) - bits_to_read);
    
    // Update position
    uint8_t next_bit_pos = pbbr->bit_pos + bits_to_read;
    pbbr->byte_pos += next_bit_pos >> 3;
    pbbr->bit_pos = next_bit_pos & 7;
    *read_bit_len -= bits_to_read;
    
    return result;
}
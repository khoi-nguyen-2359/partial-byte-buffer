#ifndef PARTIAL_BYTE_BUFFER_READER_H
#define PARTIAL_BYTE_BUFFER_READER_H

#include <stdint.h>
#include <stddef.h>

typedef struct PartialByteBufferReader {
    uint8_t* buffer;
    size_t length;
    size_t byte_pos;
    uint8_t bit_pos;
} PartialByteBufferReader;

PartialByteBufferReader* pbbr_create(const uint8_t* buffer, size_t length);
void pbbr_destroy(PartialByteBufferReader** pbbr);

int8_t pbbr_read_byte(PartialByteBufferReader* pbbr, uint8_t bit_len);
int pbbr_read_int(PartialByteBufferReader* pbbr, uint8_t bit_len);

#endif // PARTIAL_BYTE_BUFFER_READER_H
#ifndef PARTIAL_BYTE_BUFFER_READER_H
#define PARTIAL_BYTE_BUFFER_READER_H

#include <stdint.h>
#include <stddef.h>

typedef struct PartialByteBufferReader {
    const int8_t* buffer;
    size_t length;
    size_t cursor;
} PartialByteBufferReader;

PartialByteBufferReader* pbbr_create(const int8_t* buffer, size_t length);
void pbbr_destroy(PartialByteBufferReader* pbbr);

#endif // PARTIAL_BYTE_BUFFER_READER_H
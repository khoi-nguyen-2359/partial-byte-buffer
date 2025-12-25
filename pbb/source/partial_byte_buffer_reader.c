#include "partial_byte_buffer_reader.h"

PartialByteBufferReader* pbbr_create(const int8_t* buffer, size_t length) {
    PartialByteBufferReader* pbbr = (PartialByteBufferReader*)malloc(sizeof(PartialByteBufferReader));
    if (pbbr != NULL) {
        int8_t* buffer_copy = (int8_t*)malloc(length);
        if (buffer_copy != NULL) {
            memcpy(buffer_copy, buffer, length);
            pbbr->buffer = buffer_copy;
            pbbr->length = length;
            pbbr->cursor = 0;
        } else {
            free(pbbr);
            pbbr = NULL;
        }
    }
    return pbbr;
}

void pbbr_destroy(PartialByteBufferReader* pbbr) {
    if (pbbr != NULL) {
        free(pbbr->buffer);
        free(pbbr);
        pbbr = NULL;
    }
}
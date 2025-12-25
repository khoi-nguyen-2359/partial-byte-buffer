#ifndef PARTIAL_BYTE_BUFFER_H
#define PARTIAL_BYTE_BUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef struct PartialByteBuffer {
    uint8_t* buffer;
    uint32_t capacity;
    uint32_t byte_pos;
    uint8_t bit_pos;
} PartialByteBuffer;

PartialByteBuffer* pbb_create(uint32_t initial_capacity);
void pbb_destroy(PartialByteBuffer* pbb);

void pbb_put_byte(PartialByteBuffer* pbb, int8_t byte, uint8_t bit_len);
void pbb_put_int(PartialByteBuffer* pbb, int value, uint8_t bit_len);
void pbb_put_long(PartialByteBuffer* pbb, int64_t value, int bit_len);

int8_t* pbb_to_byte_array(const PartialByteBuffer* pbb, size_t* out_size);

#endif // FRACTIONAL_BYTE_BUFFER_H

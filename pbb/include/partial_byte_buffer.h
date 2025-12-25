#ifndef PARTIAL_BYTE_BUFFER_H
#define PARTIAL_BYTE_BUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef struct PartialByteBuffer {
    int8_t* buffer;
    size_t capacity;
    size_t byte_pos;
    uint8_t bit_pos;
} PartialByteBuffer;

PartialByteBuffer* pbb_create(int initial_capacity);  // Change parameter
void pbb_destroy(PartialByteBuffer* pbb);
size_t pbb_get_length(const PartialByteBuffer* pbb);

void pbb_put_byte(PartialByteBuffer* pbb, int8_t byte, uint8_t bit_len);
void pbb_put_int(PartialByteBuffer* pbb, int value, uint8_t bit_len);
void pbb_put_long(PartialByteBuffer* pbb, long value, uint8_t bit_len);

int8_t* pbb_to_byte_array(const PartialByteBuffer* pbb, size_t* out_size);
int8_t* pbb_get_buffer_array(const PartialByteBuffer* pbb, size_t* out_size);

#endif // PARTIAL_BYTE_BUFFER_H

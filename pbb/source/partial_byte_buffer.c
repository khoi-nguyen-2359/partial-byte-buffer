#include "partial_byte_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define MIN_INIT_LEN 2
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(val, min, max) ( (val) < (min) ? (min) : ( (val) > (max) ? (max) : (val) ) )

#define BITSIZEOF_UINT (sizeof(unsigned int) * 8)
#define BITSIZEOF_INT (sizeof(int) * 8)
#define BITSIZEOF_LONG (sizeof(long) * 8)

static void put_byte(PartialByteBuffer* pbb, unsigned int value, uint8_t* value_bit_len, uint8_t put_bit_len);
static unsigned int highest_one_bit(int value);
static size_t next_capacity(size_t n);
static void ensure_capacity(PartialByteBuffer* pbb, uint8_t bit_len);

PartialByteBuffer* pbb_create(int initial_capacity) {
    if (initial_capacity <= 0) return NULL;
    
    PartialByteBuffer* pbb = (PartialByteBuffer*)malloc(sizeof(PartialByteBuffer));
    if (pbb != NULL) {
        size_t capacity = 1U << highest_one_bit(initial_capacity);
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
    if (pbb != NULL) {
        free((*pbb)->buffer);
        free(*pbb);
        *pbb = NULL;
    }
}

size_t pbb_get_length(const PartialByteBuffer* pbb) {
    if (pbb == NULL) return 0;
    return pbb->byte_pos + ((pbb->bit_pos + 7) >> 3);
}

void pbb_put_byte(PartialByteBuffer* pbb, int8_t byte, uint8_t bit_len) {
    if (pbb == NULL || bit_len <= 0 || bit_len > 8) return;

    ensure_capacity(pbb, bit_len);

    uint8_t remaining_bit_len = bit_len;
    put_byte(pbb, byte, &remaining_bit_len, 8 - pbb->bit_pos);
    put_byte(pbb, byte, &remaining_bit_len, 8);
}

void pbb_put_int(PartialByteBuffer* pbb, int value, uint8_t bit_len) {
    if (pbb == NULL || bit_len <= 0 || bit_len > BITSIZEOF_INT) return;

    ensure_capacity(pbb, bit_len);
    
    uint8_t remaining_bit_len = bit_len;
    put_byte(pbb, value, &remaining_bit_len, 8 - pbb->bit_pos);
    for (int i = 0; i < 3; ++i) {
        put_byte(pbb, value, &remaining_bit_len, 8);
    }
}

uint8_t* pbb_to_byte_array(const PartialByteBuffer* pbb, size_t* out_size) {
    if (pbb == NULL) return NULL;

    size_t byte_count = pbb_get_length(pbb);
    uint8_t* result = (uint8_t*)malloc(byte_count);
    if (result != NULL) {
        memcpy(result, pbb->buffer, byte_count);
        if (out_size != NULL) {
            *out_size = byte_count;
        }
    }
    return result;
}

uint8_t* pbb_get_buffer_array(const PartialByteBuffer* pbb, size_t* out_size) {
    if (pbb == NULL) return NULL;

    *out_size = pbb_get_length(pbb);
    return pbb->buffer;
}

static void put_byte(PartialByteBuffer* pbb, unsigned int value, uint8_t* value_bit_len, uint8_t available_bit_len) {
    if (*value_bit_len <= 0) return;

    // printf("Putting bits: value=0x%X, value_bit_len=%d, available_bit_len=%d\n", value, *value_bit_len, available_bit_len);
    uint8_t put_bit_len = MIN(available_bit_len, *value_bit_len);
    pbb->buffer[pbb->byte_pos] |= value << (BITSIZEOF_UINT - *value_bit_len) >> (BITSIZEOF_UINT - put_bit_len) << (8 - pbb->bit_pos - put_bit_len);
    uint8_t next_bit_pos = pbb->bit_pos + put_bit_len;
    pbb->byte_pos += next_bit_pos >> 3;
    pbb->bit_pos = next_bit_pos & 7;
    *value_bit_len -= put_bit_len;
}

static unsigned int highest_one_bit(int value) {
    if (value == 0) return 0;

    unsigned int position = 0;
    
    if (value >= 0x10000) { position += 16; value >>= 16; }
    if (value >= 0x100)   { position += 8;  value >>= 8; }
    if (value >= 0x10)    { position += 4;  value >>= 4; }
    if (value >= 0x4)     { position += 2;  value >>= 2; }
    if (value >= 0x2)     { position += 1; }

    return position;
}

static size_t next_capacity(size_t n) {
    size_t next_power2 = n << 1;
    if (next_power2 <= n) {
        return n | n >> 1;
    }
    return next_power2;
}

static void ensure_capacity(PartialByteBuffer* pbb, uint8_t bit_len) {
    size_t required_bytes = pbb->byte_pos + ((pbb->bit_pos + bit_len) >> 3);
    // printf("Ensuring capacity: required_bytes=%zu, current_capacity=%zu\n", required_bytes, pbb->capacity);
    if (required_bytes <= pbb->capacity)
        return;

    size_t capacity = next_capacity(pbb->capacity);
    uint8_t* new_buffer = (uint8_t*)realloc(pbb->buffer, capacity);
    if (new_buffer != NULL) {   
        memset(new_buffer + pbb->capacity, 0, capacity - pbb->capacity);
        pbb->buffer = new_buffer;
        pbb->capacity = capacity;
    }
}
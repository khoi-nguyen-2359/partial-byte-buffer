#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb = nullptr;
        }
};

#pragma region CREATE - DESTROY TESTS

TEST_F(PartialByteBufferTest, Create_CorrectAllocation_CorrectCursors) {
    pbb = pbb_create(10);
    ASSERT_NE(pbb, nullptr);
    ASSERT_EQ(pbb->capacity, 16);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, Create_ZeroCapacity_NothingAllocated) {
    pbb = pbb_create(0);
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferTest, Create_NegativeCapacity_NothingAllocated) {
    pbb = pbb_create(-1);
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferTest, Destroy_NoCrash) {
    pbb = pbb_create(1);
    ASSERT_NE(pbb, nullptr);
    pbb_destroy(&pbb);
    SUCCEED();
}

#pragma endregion

#pragma region WRITE BYTE TESTS

TEST_F(PartialByteBufferTest, WriteByte_SingleByte_CorrectValue) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0x42, 8);
    ASSERT_EQ(pbb->buffer[0], 0x42);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_MultipleBytes_CorrectValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_byte(pbb, 0x22, 8);
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->byte_pos, 2);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_NullBuffer_DoNothing) {
    partial_byte_buffer* pbb_ptr = nullptr;
    pbb_write_byte(pbb_ptr, 0x42, 8);
    ASSERT_EQ(pbb_ptr, nullptr);
}

TEST_F(PartialByteBufferTest, WriteByte_PartialByteOnce_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 3);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_PartialByteTwice_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 3);

    pbb_write_byte(pbb, 0b0100110, 7);
    ASSERT_EQ(pbb->buffer[0], 0b10101001);
    ASSERT_EQ(pbb->buffer[1], 0b10000000);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_PartialThenFullByte_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 3);

    pbb_write_byte(pbb, 0b11111, 5);
    ASSERT_EQ(pbb->buffer[0], 0b10111111);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 0);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_MultipleSingleBits_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 1, 1);
    pbb_write_byte(pbb, 0, 1);
    pbb_write_byte(pbb, 1, 1);
    pbb_write_byte(pbb, 1, 1);
    // Result: 0b1011
    ASSERT_EQ(pbb->buffer[0], 0b10110000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 4);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_NegativePartialByte_CorrectValue) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, -3, 5);
    ASSERT_EQ(pbb->buffer[0], 0xE8);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 5);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_NegativeFullByte_CorrectValue) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, -3, 8);
    ASSERT_EQ(pbb->buffer[0], 0xFD);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region WRITE INT TESTS

TEST_F(PartialByteBufferTest, WriteInt_FullInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0x11223344, 32);

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->byte_pos, 4);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteInt_MultipleInts_CorrectBufferValuesAndPositions) {
    pbb = pbb_create(8);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0x11223344, 32);
    pbb_write_int(pbb, 0x55667788, 32);

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->buffer[4], 0x55);
    ASSERT_EQ(pbb->buffer[5], 0x66);
    ASSERT_EQ(pbb->buffer[6], 0x77);
    ASSERT_EQ(pbb->buffer[7], 0x88);
    ASSERT_EQ(pbb->byte_pos, 8);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteInt_PartialInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0xABC, 12);

    ASSERT_EQ(pbb->buffer[0], 0xAB);
    ASSERT_EQ(pbb->buffer[1], 0xC0);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteInt_NegativePartialInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0xF6B, 12); // -149

    ASSERT_EQ(pbb->buffer[0], 0xF6);
    ASSERT_EQ(pbb->buffer[1], 0xB0);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteInt_NegativeFullInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0xD1ECFE96, 32); // -772,997,482

    ASSERT_EQ(pbb->buffer[0], 0xD1);
    ASSERT_EQ(pbb->buffer[1], 0xEC);
    ASSERT_EQ(pbb->buffer[2], 0xFE);
    ASSERT_EQ(pbb->buffer[3], 0x96);
    ASSERT_EQ(pbb->byte_pos, 4);
    ASSERT_EQ(pbb->bit_pos, 0);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteInt_NullBuffer_DoNothing) {
    partial_byte_buffer* pbb_ptr = nullptr;
    pbb_write_int(pbb_ptr, 0x12345678, 32);
    ASSERT_EQ(pbb_ptr, nullptr);
}

#pragma endregion

#pragma region WRITE COMBINATION TESTS

TEST_F(PartialByteBufferTest, WriteByteThenInt_CorrectBufferValues) {
    pbb = pbb_create(6);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0xAA, 8);
    pbb_write_int(pbb, 0x11223344, 32);

    ASSERT_EQ(pbb->buffer[0], 0xAA);
    ASSERT_EQ(pbb->buffer[1], 0x11);
    ASSERT_EQ(pbb->buffer[2], 0x22);
    ASSERT_EQ(pbb->buffer[3], 0x33);
    ASSERT_EQ(pbb->buffer[4], 0x44);
    ASSERT_EQ(pbb->byte_pos, 5);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutPartialByteThenPartialInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0b101, 3);      
    pbb_write_int(pbb, 0b101010111100, 12);

    ASSERT_EQ(pbb->buffer[0], 0b10110101);
    ASSERT_EQ(pbb->buffer[1], 0b01111000);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 7);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region CAPACITY EXPANSION TESTS

TEST_F(PartialByteBufferTest, WriteByte_ExceedCapacityOnce_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_byte(pbb, 0x22, 8);
    pbb_write_byte(pbb, 0x33, 8);  // This should exceed capacity
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->byte_pos, 3);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_ExceedCapacityTwice_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_byte(pbb, 0x22, 8);
    pbb_write_byte(pbb, 0x33, 8);  // This should exceed capacity
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->byte_pos, 3);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_write_byte(pbb, 0x44, 8);
    pbb_write_byte(pbb, 0x55, 8);  // This should exceed capacity again
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->buffer[4], 0x55);
    ASSERT_EQ(pbb->byte_pos, 5);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}


TEST_F(PartialByteBufferTest, WriteInt_ExceedCapacityOnce_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0x11223344, 32); // requires growth past initial 2 bytes

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_GE(pbb->capacity, 4);
    ASSERT_EQ(pbb->byte_pos, 4);
    ASSERT_EQ(pbb->bit_pos, 0);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutPartialByteThenPartialInt_ExceedCapacity_CorrectBufferValuesAndCapacity) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0b10100, 5);      
    pbb_write_int(pbb, 0b101010111101, 12);

    ASSERT_EQ(pbb->buffer[0], 0b10100101);
    ASSERT_EQ(pbb->buffer[1], 0b01011110);
    ASSERT_EQ(pbb->buffer[2], 0b10000000);
    ASSERT_EQ(pbb->byte_pos, 2);
    ASSERT_EQ(pbb->bit_pos, 1);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}


TEST_F(PartialByteBufferTest, WriteByteThenInt_ExceedCapacityTwiceAtOnce_CorrectBufferCapacity) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22334455, 32); // Exceeds initial capacity from 2 -> 8
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region GET LENGTH TESTS

TEST_F(PartialByteBufferTest, GetLength_EmptyBuffer_ZeroLength) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 0);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetLength_BufferChangedWithoutExceedingCapacity_CorrectLength) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22, 8);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(pbb->capacity, 4);
    ASSERT_EQ(length, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetLength_BufferChangedExceedingCapacity_CorrectLength) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22334455, 32); // Exceeds initial capacity
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(pbb->capacity, 8);
    ASSERT_EQ(length, 5);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetLength_PuttingPartialBytes_RoundUpCorrectly) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0b101, 3);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 1);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetLength_PuttingPartialBytesExceedingCapacity_RoundUpCorrectly) {
    pbb = pbb_create(2);

    pbb_write_int(pbb, 0b10100011010011001, 17);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 3);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region TO BUFFER ARRAY TESTS

TEST_F(PartialByteBufferTest, ToBufferArray_NullPbb_NullReturned) {
    pbb = nullptr;

    size_t out_size = -1;
    uint8_t* buffer_array = pbb_to_byte_array(pbb, &out_size);
    ASSERT_EQ(buffer_array, nullptr);
    ASSERT_EQ(out_size, 0);
}

TEST_F(PartialByteBufferTest, ToBufferArray_EmptyBuffer_NullReturned) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    size_t out_size = -1;
    uint8_t* buffer_array = pbb_to_byte_array(pbb, &out_size);
    ASSERT_EQ(buffer_array, nullptr);
    ASSERT_EQ(out_size, 0);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, ToBufferArray_NonEmptyBuffer_CorrectContentAndSize) {
    pbb = pbb_create(8);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0x12, 8);
    pbb_write_int(pbb, 0x3456789A, 32);

    size_t out_size = -1;
    uint8_t* buffer_array = pbb_to_byte_array(pbb, &out_size);
    ASSERT_NE(buffer_array, nullptr);
    ASSERT_EQ(out_size, 5);
    ASSERT_EQ(buffer_array[0], 0x12);
    ASSERT_EQ(buffer_array[1], 0x34);
    ASSERT_EQ(buffer_array[2], 0x56);
    ASSERT_EQ(buffer_array[3], 0x78);
    ASSERT_EQ(buffer_array[4], 0x9A);

    free(buffer_array);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, ToBufferArray_BufferHasExtended_CorrectContentAndSize) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0x12, 8);
    pbb_write_int(pbb, 0x3456789A, 32);

    size_t out_size = -1;
    uint8_t* buffer_array = pbb_to_byte_array(pbb, &out_size);
    ASSERT_NE(buffer_array, nullptr);
    ASSERT_EQ(out_size, 5);
    ASSERT_EQ(buffer_array[0], 0x12);
    ASSERT_EQ(buffer_array[1], 0x34);
    ASSERT_EQ(buffer_array[2], 0x56);
    ASSERT_EQ(buffer_array[3], 0x78);
    ASSERT_EQ(buffer_array[4], 0x9A);

    free(buffer_array);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, ToByteArray_NullOutSize_DoesNotCrash) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0x12, 8);

    uint8_t* buffer_array = pbb_to_byte_array(pbb, nullptr);
    ASSERT_NE(buffer_array, nullptr);
    ASSERT_EQ(buffer_array[0], 0x12);

    free(buffer_array);
    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region GET BUFFER ARRAY TESTS

TEST_F(PartialByteBufferTest, GetBufferArray_NullPbb_NullReturned) {
    pbb = nullptr;

    size_t out_size = -1;
    uint8_t* buffer_array = pbb_get_buffer_array(pbb, &out_size);
    ASSERT_EQ(buffer_array, nullptr);
    ASSERT_EQ(out_size, 0);
}

TEST_F(PartialByteBufferTest, GetBufferArray_EmptyBuffer_CorrectSize) {
    pbb = pbb_create(4);

    size_t out_size = -1;
    uint8_t* buffer_array = pbb_get_buffer_array(pbb, &out_size);
    ASSERT_EQ(buffer_array, pbb->buffer);
    ASSERT_EQ(out_size, 0);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetBufferArray_NonEmptyBuffer_CorrectContentAndSize) {
    pbb = pbb_create(8);

    pbb_write_byte(pbb, 0x12, 8);
    pbb_write_int(pbb, 0x3456789A, 32);

    size_t out_size = -1;
    uint8_t* buffer_array = pbb_get_buffer_array(pbb, &out_size);
    ASSERT_EQ(buffer_array, pbb->buffer);
    ASSERT_EQ(out_size, 5);
    ASSERT_EQ(buffer_array[0], 0x12);
    ASSERT_EQ(buffer_array[1], 0x34);
    ASSERT_EQ(buffer_array[2], 0x56);
    ASSERT_EQ(buffer_array[3], 0x78);
    ASSERT_EQ(buffer_array[4], 0x9A);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetBufferArray_NullOutSize_DoesNotCrash) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0x12, 8);

    uint8_t* buffer_array = pbb_get_buffer_array(pbb, nullptr);
    ASSERT_EQ(buffer_array, pbb->buffer);
    ASSERT_EQ(buffer_array[0], 0x12);

    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region EDGE CASE TESTS

TEST_F(PartialByteBufferTest, WriteByte_InvalidBitLength_DoesNothing) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0xAB, 0);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_write_byte(pbb, 0xAB, 9);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteInt_InvalidBitLength_DoesNothing) {
    pbb = pbb_create(2);
    
    pbb_write_int(pbb, 0x12345678, 0);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_write_int(pbb, 0x12345678, 33);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_ManyTimesWithInvalidBitLength_BufferIndicesCorrect) {
    pbb = pbb_create(2);

    const int total_bytes = 1000;
    const int random_seed = 42;
    srand(random_seed);
    const int rand_bits[] = {-8,-7,-6,-5,-4,-3,-2,-1,1,2,3,4,5,6,7,8};

    int bit_count = 0;
    int capacity = 2;
    for (int i = 0; i < total_bytes; ++i) {
        int rand_bit_len = rand_bits[rand() % (sizeof(rand_bits)/sizeof(rand_bits[0]))];
        pbb_write_byte(pbb, 0xAB, rand_bit_len);
        bit_count += rand_bit_len > 0 ? rand_bit_len : 0;
        if (bit_count > capacity * 8) {
            capacity *= 2;
        }
    }

    ASSERT_EQ(pbb->byte_pos, bit_count >> 3);
    ASSERT_EQ(pbb->bit_pos, bit_count & 7);
    ASSERT_GE(pbb->capacity, capacity);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, WriteByte_ManyTimesFullByte_BufferContentAndIndicesCorrect) {
    pbb = pbb_create(2);

    const int total_bytes = 1000;
    for (int i = 0; i < total_bytes; ++i) {
        pbb_write_byte(pbb, 0xAB, 8);
    }

    int bit_count = total_bytes * 8;
    ASSERT_EQ(pbb->byte_pos, bit_count >> 3);
    ASSERT_EQ(pbb->bit_pos, bit_count & 7);
    ASSERT_GE(pbb->capacity, 1024);

    for (int i = 0; i < total_bytes; ++i) {
        ASSERT_EQ(pbb->buffer[i], 0xAB);
    }

    pbb_destroy(&pbb);
}

#pragma endregion

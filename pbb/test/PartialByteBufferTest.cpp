#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferTest : public ::testing::Test {
    protected:
        PartialByteBuffer *pbb = nullptr;
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

#pragma region PUT BYTE TESTS

TEST_F(PartialByteBufferTest, PutByte_SingleByte_CorrectValue) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0x42, 8);
    ASSERT_EQ(pbb->buffer[0], 0x42);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutByte_MultipleBytes_CorrectValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0x11, 8);
    pbb_put_byte(pbb, 0x22, 8);
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->byte_pos, 2);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutByte_NullBuffer_DoNothing) {
    PartialByteBuffer* pbb_ptr = nullptr;
    pbb_put_byte(pbb_ptr, 0x42, 8);
    ASSERT_EQ(pbb_ptr, nullptr);
}

TEST_F(PartialByteBufferTest, PutByte_PartialByteOnce_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 3);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutByte_PartialByteTwice_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 3);

    pbb_put_byte(pbb, 0b0100110, 7);
    ASSERT_EQ(pbb->buffer[0], 0b10101001);
    ASSERT_EQ(pbb->buffer[1], 0b10000000);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutByte_PartialThenFullByte_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 3);

    pbb_put_byte(pbb, 0b11111, 5);
    ASSERT_EQ(pbb->buffer[0], 0b10111111);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 0);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutByte_MultipleSingleBits_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 1, 1);
    pbb_put_byte(pbb, 0, 1);
    pbb_put_byte(pbb, 1, 1);
    pbb_put_byte(pbb, 1, 1);
    // Result: 0b1011
    ASSERT_EQ(pbb->buffer[0], 0b10110000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 4);
    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region PUT INT TESTS

TEST_F(PartialByteBufferTest, PutInt_FullInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_put_int(pbb, 0x11223344, 32);

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->byte_pos, 4);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutInt_MultipleInts_CorrectBufferValuesAndPositions) {
    pbb = pbb_create(8);
    ASSERT_NE(pbb, nullptr);

    pbb_put_int(pbb, 0x11223344, 32);
    pbb_put_int(pbb, 0x55667788, 32);

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

TEST_F(PartialByteBufferTest, PutInt_PartialInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_put_int(pbb, 0xABC, 12);

    ASSERT_EQ(pbb->buffer[0], 0xAB);
    ASSERT_EQ(pbb->buffer[1], 0xC0);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutInt_NullBuffer_DoNothing) {
    PartialByteBuffer* pbb_ptr = nullptr;
    pbb_put_int(pbb_ptr, 0x12345678, 32);
    ASSERT_EQ(pbb_ptr, nullptr);
}



#pragma endregion

#pragma region PUT COMBINATION TESTS

TEST_F(PartialByteBufferTest, PutByteThenInt_CorrectBufferValues) {
    pbb = pbb_create(6);
    ASSERT_NE(pbb, nullptr);

    pbb_put_byte(pbb, 0xAA, 8);
    pbb_put_int(pbb, 0x11223344, 32);

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

    pbb_put_byte(pbb, 0b101, 3);      
    pbb_put_int(pbb, 0b101010111100, 12);

    ASSERT_EQ(pbb->buffer[0], 0b10110101);
    ASSERT_EQ(pbb->buffer[1], 0b01111000);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 7);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region CAPACITY EXPANSION TESTS

TEST_F(PartialByteBufferTest, PutByte_ExceedCapacityOnce_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0x11, 8);
    pbb_put_byte(pbb, 0x22, 8);
    pbb_put_byte(pbb, 0x33, 8);  // This should exceed capacity
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->byte_pos, 3);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, PutByte_ExceedCapacityTwice_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0x11, 8);
    pbb_put_byte(pbb, 0x22, 8);
    pbb_put_byte(pbb, 0x33, 8);  // This should exceed capacity
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->byte_pos, 3);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_put_byte(pbb, 0x44, 8);
    pbb_put_byte(pbb, 0x55, 8);  // This should exceed capacity again
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->buffer[4], 0x55);
    ASSERT_EQ(pbb->byte_pos, 5);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}


TEST_F(PartialByteBufferTest, PutInt_ExceedCapacityOnce_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_put_int(pbb, 0x11223344, 32); // requires growth past initial 2 bytes

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

    pbb_put_byte(pbb, 0b10100, 5);      
    pbb_put_int(pbb, 0b101010111101, 12);

    ASSERT_EQ(pbb->buffer[0], 0b10100101);
    ASSERT_EQ(pbb->buffer[1], 0b01011110);
    ASSERT_EQ(pbb->buffer[2], 0b10000000);
    ASSERT_EQ(pbb->byte_pos, 2);
    ASSERT_EQ(pbb->bit_pos, 1);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}


TEST_F(PartialByteBufferTest, PutByteThenInt_ExceedCapacityFourTimesAtOnce_CorrectBufferCapacity) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_put_byte(pbb, 0x11, 8);
    pbb_put_int(pbb, 0x22334455, 32); // Exceeds initial capacity from 2 -> 8
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

    pbb_put_byte(pbb, 0x11, 8);
    pbb_put_int(pbb, 0x22, 8);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(pbb->capacity, 4);
    ASSERT_EQ(length, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetLength_BufferChangedExceedingCapacity_CorrectLength) {
    pbb = pbb_create(2);

    pbb_put_byte(pbb, 0x11, 8);
    pbb_put_int(pbb, 0x22334455, 32); // Exceeds initial capacity
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(pbb->capacity, 8);
    ASSERT_EQ(length, 5);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetLength_PuttingPartialBytes_CorrectLength) {
    pbb = pbb_create(2);

    pbb_put_byte(pbb, 0b101, 3);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 1);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferTest, GetLength_PuttingPartialBytesExceedingCapacity_CorrectLength) {
    pbb = pbb_create(2);

    pbb_put_int(pbb, 0b10100011010011001, 17);
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

    pbb_put_byte(pbb, 0x12, 8);
    pbb_put_int(pbb, 0x3456789A, 32);

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

    pbb_put_byte(pbb, 0x12, 8);
    pbb_put_int(pbb, 0x3456789A, 32);

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



#pragma endregion
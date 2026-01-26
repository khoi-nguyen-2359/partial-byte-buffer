#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferWriteTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(&pbb);
        }
};

#pragma region WRITE COMBINATION TESTS

TEST_F(PartialByteBufferWriteTest, WriteFullNumbers_CorrectBufferValues) {
    pbb = pbb_create(13);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22334455, 32);
    pbb_write_int64(pbb, 0x66778899AABBCCDD, 64);

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->buffer[4], 0x55);
    ASSERT_EQ(pbb->buffer[5], 0x66);
    ASSERT_EQ(pbb->buffer[6], 0x77);
    ASSERT_EQ(pbb->buffer[7], 0x88);
    ASSERT_EQ(pbb->buffer[8], 0x99);
    ASSERT_EQ(pbb->buffer[9], 0xAA);
    ASSERT_EQ(pbb->buffer[10], 0xBB);
    ASSERT_EQ(pbb->buffer[11], 0xCC);   
    ASSERT_EQ(pbb->buffer[12], 0xDD);
    ASSERT_EQ(pbb->write_pos, 104);
    ASSERT_EQ(pbb->capacity, 13);
}

TEST_F(PartialByteBufferWriteTest, WritePartialNumbers_CorrectBufferValues) {
    pbb = pbb_create(7);

    pbb_write_byte(pbb, 0b101, 3);      
    pbb_write_int(pbb, 0b101010111100, 12);
    pbb_write_int64(pbb, 0b111100001111000011110000111100001010, 36);

    ASSERT_EQ(pbb->buffer[0], 0b10110101);
    ASSERT_EQ(pbb->buffer[1], 0b01111001);
    ASSERT_EQ(pbb->buffer[2], 0b11100001);
    ASSERT_EQ(pbb->buffer[3], 0b11100001);
    ASSERT_EQ(pbb->buffer[4], 0b11100001);
    ASSERT_EQ(pbb->buffer[5], 0b11100001);
    ASSERT_EQ(pbb->buffer[6], 0b01000000);
    ASSERT_EQ(pbb->write_pos, 51);
    ASSERT_EQ(pbb->capacity, 7);
}

#pragma endregion

#pragma region GET LENGTH TESTS

TEST_F(PartialByteBufferWriteTest, GetLength_EmptyBuffer_ZeroLength) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 0);
}

TEST_F(PartialByteBufferWriteTest, GetLength_BufferChangedWithoutExceedingCapacity_CorrectLength) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22, 8);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(pbb->capacity, 4);
    ASSERT_EQ(length, 2);
}

TEST_F(PartialByteBufferWriteTest, GetLength_BufferChangedExceedingCapacity_CorrectLength) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22334455, 32); // Exceeds initial capacity
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 5);
}

TEST_F(PartialByteBufferWriteTest, GetLength_PuttingPartialBytes_RoundUpCorrectly) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0b101, 3);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 1);
}

TEST_F(PartialByteBufferWriteTest, GetLength_PuttingPartialBytesExceedingCapacity_RoundUpCorrectly) {
    pbb = pbb_create(2);

    pbb_write_int(pbb, 0b10100011010011001, 17);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 3);
}

#pragma endregion

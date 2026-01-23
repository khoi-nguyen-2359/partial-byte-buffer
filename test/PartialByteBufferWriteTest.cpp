#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferWriteTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            if (pbb != nullptr) {
                pbb_destroy(&pbb);
            }
        }
};

#pragma region CREATE - DESTROY TESTS

TEST_F(PartialByteBufferWriteTest, Create_CorrectAllocation_CorrectCursors) {
    pbb = pbb_create(10);
    ASSERT_NE(pbb, nullptr);
    ASSERT_EQ(pbb->capacity, 10);
    ASSERT_EQ(pbb->write_pos, 0);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, Create_ZeroCapacity_NothingAllocated) {
    pbb = pbb_create(0);
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferWriteTest, Create_NegativeCapacity_NothingAllocated) {
    pbb = pbb_create(-1);
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferWriteTest, Destroy_NoCrash) {
    pbb = pbb_create(1);
    ASSERT_NE(pbb, nullptr);
    pbb_destroy(&pbb);
    SUCCEED();
}

#pragma endregion

#pragma region WRITE BYTE TESTS

TEST_F(PartialByteBufferWriteTest, WriteByte_SingleByte_CorrectValue) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0x42, 8);
    ASSERT_EQ(pbb->buffer[0], 0x42);
    ASSERT_EQ(pbb->write_pos, 8);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_MultipleBytes_CorrectValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_byte(pbb, 0x22, 8);
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->write_pos, 16);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_NullBuffer_DoNothing) {
    partial_byte_buffer* pbb_ptr = nullptr;
    pbb_write_byte(pbb_ptr, 0x42, 8);
    ASSERT_EQ(pbb_ptr, nullptr);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_PartialByteOnce_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->write_pos, 3);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_PartialByteTwice_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->write_pos, 3);

    pbb_write_byte(pbb, 0b0100110, 7);
    ASSERT_EQ(pbb->buffer[0], 0b10101001);
    ASSERT_EQ(pbb->buffer[1], 0b10000000);
    ASSERT_EQ(pbb->write_pos, 10);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_PartialThenFullByte_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->write_pos, 3);

    pbb_write_byte(pbb, 0b11111, 5);
    ASSERT_EQ(pbb->buffer[0], 0b10111111);
    ASSERT_EQ(pbb->write_pos, 8);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_MultipleSingleBits_CorrectBufferValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 1, 1);
    pbb_write_byte(pbb, 0, 1);
    pbb_write_byte(pbb, 1, 1);
    pbb_write_byte(pbb, 1, 1);
    // Result: 0b1011
    ASSERT_EQ(pbb->buffer[0], 0b10110000);
    ASSERT_EQ(pbb->write_pos, 4);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_NegativePartialByte_CorrectValue) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, -3, 5);
    ASSERT_EQ(pbb->buffer[0], 0xE8);
    ASSERT_EQ(pbb->write_pos, 5);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_NegativeFullByte_CorrectValue) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, -3, 8);
    ASSERT_EQ(pbb->buffer[0], 0xFD);
    ASSERT_EQ(pbb->write_pos, 8);
    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region WRITE INT TESTS

TEST_F(PartialByteBufferWriteTest, WriteInt_FullInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0x11223344, 32);

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->write_pos, 32);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt_MultipleInts_CorrectBufferValuesAndPositions) {
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
    ASSERT_EQ(pbb->write_pos, 64);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt_PartialInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0xABC, 12);

    ASSERT_EQ(pbb->buffer[0], 0xAB);
    ASSERT_EQ(pbb->buffer[1], 0xC0);
    ASSERT_EQ(pbb->write_pos, 12);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt_NegativePartialInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0xF6B, 12); // -149

    ASSERT_EQ(pbb->buffer[0], 0xF6);
    ASSERT_EQ(pbb->buffer[1], 0xB0);
    ASSERT_EQ(pbb->write_pos, 12);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt_NegativeFullInt_CorrectBufferValues) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0xD1ECFE96, 32); // -772,997,482

    ASSERT_EQ(pbb->buffer[0], 0xD1);
    ASSERT_EQ(pbb->buffer[1], 0xEC);
    ASSERT_EQ(pbb->buffer[2], 0xFE);
    ASSERT_EQ(pbb->buffer[3], 0x96);
    ASSERT_EQ(pbb->write_pos, 32);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt_NullBuffer_DoNothing) {
    partial_byte_buffer* pbb_ptr = nullptr;
    pbb_write_int(pbb_ptr, 0x12345678, 32);
    ASSERT_EQ(pbb_ptr, nullptr);
}

#pragma endregion

#pragma region WRITE INT 64 TESTS

TEST_F(PartialByteBufferWriteTest, WriteInt64_FullInt64_CorrectBufferValues) {
    pbb = pbb_create(8);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int64(pbb, 0x1122334455667788, 64);

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->buffer[4], 0x55);
    ASSERT_EQ(pbb->buffer[5], 0x66);
    ASSERT_EQ(pbb->buffer[6], 0x77);
    ASSERT_EQ(pbb->buffer[7], 0x88);
    ASSERT_EQ(pbb->write_pos, 64);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt64_MultipleInt64s_CorrectBufferValuesAndPositions) {
    pbb = pbb_create(16);

    pbb_write_int64(pbb, 0x1122334455667788, 64);
    pbb_write_int64(pbb, 0x99AABBCCDDEEFF00, 64);

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
    ASSERT_EQ(pbb->buffer[13], 0xEE);
    ASSERT_EQ(pbb->buffer[14], 0xFF);
    ASSERT_EQ(pbb->buffer[15], 0x00);
    ASSERT_EQ(pbb->write_pos, 128);
    ASSERT_EQ(pbb->capacity, 16);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt64_PartialInt64_Below32Bits_CorrectBufferValues) {
    pbb = pbb_create(3);

    pbb_write_int64(pbb, 0xFFFFFFFFFFABCDEF, 23);

    ASSERT_EQ(pbb->buffer[0], 0x57);
    ASSERT_EQ(pbb->buffer[1], 0x9B);
    ASSERT_EQ(pbb->buffer[2], 0xDE);
    ASSERT_EQ(pbb->write_pos, 23);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt64_PartialInt64_Above32Bits_CorrectBufferValues) {
    pbb = pbb_create(6);

    pbb_write_int64(pbb, 0x123456789A, 41);

    ASSERT_EQ(pbb->buffer[0], 0x09);
    ASSERT_EQ(pbb->buffer[1], 0x1A);
    ASSERT_EQ(pbb->buffer[2], 0x2B);
    ASSERT_EQ(pbb->buffer[3], 0x3C);
    ASSERT_EQ(pbb->buffer[4], 0x4D);
    ASSERT_EQ(pbb->buffer[5], 0x00);
    ASSERT_EQ(pbb->write_pos, 41);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt64_NullBuffer_DoNothing) {
    partial_byte_buffer* pbb_ptr = nullptr;
    pbb_write_int64(pbb_ptr, 0x123456789ABCDEF0, 64);
    ASSERT_EQ(pbb_ptr, nullptr);
}

TEST_F(PartialByteBufferWriteTest, WriteInt64_TooSmallBitLength_DoesNothing) {
    pbb = pbb_create(8);

    pbb_write_int64(pbb, 0x123456789ABCDEF0, 0);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt64_TooLargeBitLength_DoesNothing) {
    pbb = pbb_create(8);

    pbb_write_int64(pbb, 0x123456789ABCDEF0, 65);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}

#pragma endregion

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

    pbb_destroy(&pbb);
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

    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region GET LENGTH TESTS

TEST_F(PartialByteBufferWriteTest, GetLength_EmptyBuffer_ZeroLength) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 0);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, GetLength_BufferChangedWithoutExceedingCapacity_CorrectLength) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22, 8);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(pbb->capacity, 4);
    ASSERT_EQ(length, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, GetLength_BufferChangedExceedingCapacity_CorrectLength) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22334455, 32); // Exceeds initial capacity
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 5);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, GetLength_PuttingPartialBytes_RoundUpCorrectly) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0b101, 3);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 1);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, GetLength_PuttingPartialBytesExceedingCapacity_RoundUpCorrectly) {
    pbb = pbb_create(2);

    pbb_write_int(pbb, 0b10100011010011001, 17);
    size_t length = pbb_get_length(pbb);
    ASSERT_EQ(length, 3);

    pbb_destroy(&pbb);
}

#pragma endregion

#pragma region EDGE CASE TESTS

TEST_F(PartialByteBufferWriteTest, WriteByte_InvalidBitLength_DoesNothing) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0xAB, 0);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_write_byte(pbb, 0xAB, 9);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteInt_InvalidBitLength_DoesNothing) {
    pbb = pbb_create(2);
    
    pbb_write_int(pbb, 0x12345678, 0);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_write_int(pbb, 0x12345678, 33);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_ManyTimesWithInvalidBitLength_BufferIndicesCorrect) {
    pbb = pbb_create(2);

    const int total_bytes = 1000;
    const int random_seed = 42;
    srand(random_seed);
    const int rand_bits[] = {-8,-7,-6,-5,-4,-3,-2,-1,1,2,3,4,5,6,7,8};

    int bit_count = 0;
    for (int i = 0; i < total_bytes; ++i) {
        int rand_bit_len = rand_bits[rand() % (sizeof(rand_bits)/sizeof(rand_bits[0]))];
        pbb_write_byte(pbb, 0xAB, rand_bit_len);
        bit_count += rand_bit_len > 0 ? rand_bit_len : 0;
    }

    ASSERT_EQ(pbb->write_pos, bit_count);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferWriteTest, WriteByte_ManyTimesFullByte_BufferContentAndIndicesCorrect) {
    pbb = pbb_create(2);

    const int total_bytes = 1000;
    for (int i = 0; i < total_bytes; ++i) {
        pbb_write_byte(pbb, 0xAB, 8);
    }

    int bit_count = total_bytes * 8;
    ASSERT_EQ(pbb->write_pos, bit_count);
    ASSERT_GE(pbb->capacity, 1024);

    for (int i = 0; i < total_bytes; ++i) {
        ASSERT_EQ(pbb->buffer[i], 0xAB);
    }

    pbb_destroy(&pbb);
}

#pragma endregion

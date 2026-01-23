#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferReadTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(&pbb);
        }
};

#pragma region READ BYTE TESTS

TEST_F(PartialByteBufferReadTest, ReadByte_OneFullByte_CorrectValueAndCursorPositions) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0xAB, 8);
    pbb_write_byte(pbb, 0xCD, 8);

    int8_t byte1 = pbb_read_byte(pbb, 8);
    ASSERT_EQ(byte1, (int8_t)0xAB);
    ASSERT_EQ(pbb->read_pos, 8);
}

TEST_F(PartialByteBufferReadTest, ReadByte_MultipleFullBytes_CorrectValueAndCursorPositions) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0xAB, 8);
    pbb_write_byte(pbb, 0xCD, 8);

    int8_t byte1 = pbb_read_byte(pbb, 8);
    ASSERT_EQ(byte1, (int8_t)0xAB);
    ASSERT_EQ(pbb->read_pos, 8);

    int8_t byte2 = pbb_read_byte(pbb, 8);
    ASSERT_EQ(byte2, (int8_t)0xCD);
    ASSERT_EQ(pbb->read_pos, 16);
}

TEST_F(PartialByteBufferReadTest, ReadByte_PartialByteWithSignExtension_CorrectValuesAndSign) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b10100000, 8);
    pbb_write_byte(pbb, 0b00000000, 8);

    int8_t byte = pbb_read_byte(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(byte, (int8_t)0xFD); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadTest, ReadByte_PartialByteWithoutSignExtension_CorrectValuesAndSign) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b01100000, 8);
    pbb_write_byte(pbb, 0b00000000, 8);

    int8_t byte = pbb_read_byte(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(byte, (int8_t)0x3); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadTest, ReadByte_PartialBytes_CorrectValuesAndCursorPositions) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b10110101, 8);
    pbb_write_byte(pbb, 0b01111000, 8);
    ASSERT_NE(pbb, nullptr);

    int8_t value1 = pbb_read_byte(pbb, 3); // Read first 3 bits: 101
    ASSERT_EQ(value1, (int8_t)0xFD);
    ASSERT_EQ(pbb->read_pos, 3);

    int8_t value2 = pbb_read_byte(pbb, 6); // Read next 6 bits: 101010
    ASSERT_EQ(value2, (int8_t)0xEA);
    ASSERT_EQ(pbb->read_pos, 9);

    int8_t value3 = pbb_read_byte(pbb, 6); // Read next 6 bits: 111100
    ASSERT_EQ(value3, (int8_t)0xFC);
    ASSERT_EQ(pbb->read_pos, 15);
}

TEST_F(PartialByteBufferReadTest, ReadByte_ExceedBufferLength_ReturnsZeroAndStopsAtEnd) {
    pbb = pbb_create(1);
    pbb_write_byte(pbb, 0b10101011, 8);

    int8_t value1 = pbb_read_byte(pbb, 7);
    ASSERT_EQ(value1, (int8_t)0xD5);
    ASSERT_EQ(pbb->read_pos, 7);

    int8_t value2 = pbb_read_byte(pbb, 2); // Try to read beyond buffer
    ASSERT_EQ(value2, 0);
    ASSERT_EQ(pbb->read_pos, 7); // Position should not advance
}

TEST_F(PartialByteBufferReadTest, ReadByte_ZeroBitLength_ReturnsZero) {
    pbb = pbb_create(1);
    pbb_write_byte(pbb, 0xFF, 8);
    ASSERT_NE(pbb, nullptr);

    int8_t value1 = pbb_read_byte(pbb, 0); // Invalid: 0 bits
    ASSERT_EQ(value1, 0);
    ASSERT_EQ(pbb->read_pos, 0);
}

TEST_F(PartialByteBufferReadTest, ReadByte_NullReader_ReturnsZero) {
    int8_t value = pbb_read_byte(nullptr, 8);
    ASSERT_EQ(value, 0);
}

#pragma endregion

#pragma region READ INT TESTS

TEST_F(PartialByteBufferReadTest, ReadInt_OneFullInt_CorrectValueAndCursorPositions) {
    pbb = pbb_create(4);
    pbb_write_byte(pbb, 0x12, 8);
    pbb_write_byte(pbb, 0x34, 8);
    pbb_write_byte(pbb, 0x56, 8);
    pbb_write_byte(pbb, 0x78, 8);

    int value = pbb_read_int(pbb, 32);
    ASSERT_EQ(value, (int) 0x12345678);
    ASSERT_EQ(pbb->read_pos, 32);
}

TEST_F(PartialByteBufferReadTest, ReadInt_PartialIntsWithSignExtension_CorrectValuesAndSign) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b11100000, 8);
    pbb_write_byte(pbb, 0b00000000, 8);

    int value = pbb_read_int(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(value, -1); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadTest, ReadInt_PartialIntsWithoutSignExtension_CorrectValuesAndSign) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b01100000, 8);
    pbb_write_byte(pbb, 0b00000000, 8);

    int value = pbb_read_int(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(value, 3); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadTest, ReadInt_PartialIntAcrossMultiBytes_CorrectValuesAndCursors) {
    pbb = pbb_create(3);
    pbb_write_byte(pbb, 0b10110101, 8);
    pbb_write_byte(pbb, 0b11001100, 8);
    pbb_write_byte(pbb, 0b11110000, 8);

    pbb_read_int(pbb, 3); // skip the first 3 bits

    int value = pbb_read_int(pbb, 15); // Read next 15 bits: 101 0111 0011 0011
    ASSERT_EQ(value, (int)0xFFFFD733); // Sign-extended
    ASSERT_EQ(pbb->read_pos, 18);
}

TEST_F(PartialByteBufferReadTest, ReadInt_PartialIntsAlignedEnd_CorrectValuesAndCursorPositions) {
    pbb = pbb_create(4);
    pbb_write_byte(pbb, 0b10110101, 8);
    pbb_write_byte(pbb, 0b11001100, 8);
    pbb_write_byte(pbb, 0b10101010, 8);
    pbb_write_byte(pbb, 0b11110000, 8);
    ASSERT_NE(pbb, nullptr);

    int firstInt = pbb_read_int(pbb, 9); // Read first 9 bits
    ASSERT_EQ(firstInt, 0xFFFFFF6B); // Sign-extended
    ASSERT_EQ(pbb->read_pos, 9);

    int secondInt = pbb_read_int(pbb, 13); // Read next 13 bits
    ASSERT_EQ(secondInt, 0xFFFFF32A); // Sign-extended
    ASSERT_EQ(pbb->read_pos, 22);

    int thirdInt = pbb_read_int(pbb, 10); // Read remaining 10 bits
    ASSERT_EQ(thirdInt, 0xFFFFFEF0); // Sign-extended
    ASSERT_EQ(pbb->read_pos, 32);
}

TEST_F(PartialByteBufferReadTest, ReadInt_PartialIntsUnalignedEnd_CorrectValuesAndCursorPositions) {
    pbb = pbb_create(4);
    pbb_write_byte(pbb, 0b10110101, 8);
    pbb_write_byte(pbb, 0b11001100, 8);
    pbb_write_byte(pbb, 0b10101010, 8);
    pbb_write_byte(pbb, 0b11110000, 8);
    ASSERT_NE(pbb, nullptr);

    int firstInt = pbb_read_int(pbb, 9); // Read first 9 bits
    ASSERT_EQ(firstInt, 0xFFFFFF6B);
    ASSERT_EQ(pbb->read_pos, 9);

    int secondInt = pbb_read_int(pbb, 13); // Read next 13 bits
    ASSERT_EQ(secondInt, 0xFFFFF32A);
    ASSERT_EQ(pbb->read_pos, 22);

    int thirdInt = pbb_read_int(pbb, 3); // Read next 3 bits
    ASSERT_EQ(thirdInt, 0xFFFFFFFD);
    ASSERT_EQ(pbb->read_pos, 25);
}

TEST_F(PartialByteBufferReadTest, ReadInt_NegativeValues_CorrectSignExtension) {
    pbb = pbb_create(4);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);
    ASSERT_NE(pbb, nullptr);

    int firstInt = pbb_read_int(pbb, 8); // Read 8 bits with high bit set
    ASSERT_EQ(firstInt, -1);
    ASSERT_EQ(pbb->read_pos, 8);

    int secondInt = pbb_read_int(pbb, 16); // Read 16 bits all ones
    ASSERT_EQ(secondInt, -1);
    ASSERT_EQ(pbb->read_pos, 24);
}

TEST_F(PartialByteBufferReadTest, ReadInt_MixedPositiveNegative_CorrectSignExtension) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b01111111, 8);
    pbb_write_byte(pbb, 0b10000000, 8);
    ASSERT_NE(pbb, nullptr);

    int value1 = pbb_read_int(pbb, 8); // Positive: 0111 1111
    ASSERT_EQ(value1, 0x7F);
    
    int value2 = pbb_read_int(pbb, 8); // Negative: 1000 0000
    ASSERT_EQ(value2, -128);
}

TEST_F(PartialByteBufferReadTest, ReadInt_ExceedBufferLength_ReturnsZeroAndStopsAtEnd) {
    pbb = pbb_create(4);
    pbb_write_byte(pbb, 0xAB, 8);
    pbb_write_byte(pbb, 0xCD, 8);
    pbb_write_byte(pbb, 0xEF, 8);
    pbb_write_byte(pbb, 0x12, 8);
    ASSERT_NE(pbb, nullptr);

    int value1 = pbb_read_int(pbb, 31); // Read 31 bits
    ASSERT_EQ(value1, 0xD5E6F789);
    ASSERT_EQ(pbb->read_pos, 31);

    int value2 = pbb_read_int(pbb, 16); // Try to read beyond buffer
    ASSERT_EQ(value2, 0);
    ASSERT_EQ(pbb->read_pos, 31); // Position should not advance
}

TEST_F(PartialByteBufferReadTest, ReadInt_ZeroBitLength_ReturnsZero) {
    pbb = pbb_create(4);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);

    int value = pbb_read_int(pbb, 0); // Invalid: 0 bits
    ASSERT_EQ(value, 0);
    ASSERT_EQ(pbb->read_pos, 0);
}

TEST_F(PartialByteBufferReadTest, ReadInt_ExcessiveBitLength_ReturnsZero) {
    pbb = pbb_create(4);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);
    pbb_write_byte(pbb, 0xFF, 8);
    ASSERT_NE(pbb, nullptr);

    int value = pbb_read_int(pbb, 33); // Invalid: more than 32 bits
    ASSERT_EQ(value, 0);
    ASSERT_EQ(pbb->read_pos, 0);
}

TEST_F(PartialByteBufferReadTest, ReadInt_NullReader_ReturnsZero) {
    int value = pbb_read_int(nullptr, 16);
    ASSERT_EQ(value, 0);
}

TEST_F(PartialByteBufferReadTest, ReadInt_CrossByteBoundaries_CorrectValues) {
    pbb = pbb_create(4);
    pbb_write_byte(pbb, 0b11010110, 8);
    pbb_write_byte(pbb, 0b10101100, 8);
    pbb_write_byte(pbb, 0b11110000, 8);
    pbb_write_byte(pbb, 0b00001111, 8);

    int value1 = pbb_read_int(pbb, 3); // Read 3 bits: 110
    ASSERT_EQ(value1, 0xFFFFFFFE);

    int value2 = pbb_read_int(pbb, 17); // Read 17 bits crossing multiple bytes
    ASSERT_EQ(value2, 0xFFFF6ACF);
    ASSERT_EQ(pbb->read_pos, 20);
}

#pragma endregion

#include <gtest/gtest.h>

#include "partial_byte_buffer_reader.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferReaderTest : public ::testing::Test {
    protected:
        partial_byte_buffer_reader *pbbr = nullptr;
        void TearDown() override {
            if (pbbr != nullptr) {
                pbbr_destroy(&pbbr);
            }
        }
};

#pragma region CREATE - DESTROY TESTS

TEST_F(PartialByteBufferReaderTest, Create_CorrectBufferLength_CorrectCursors) {
    uint8_t test_buffer[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);
    ASSERT_EQ(pbbr->length, 4);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 0);
    ASSERT_EQ(pbbr->buffer[0], 0xAA);
    ASSERT_EQ(pbbr->buffer[1], 0xBB);
    ASSERT_EQ(pbbr->buffer[2], 0xCC);
    ASSERT_EQ(pbbr->buffer[3], 0xDD);
    
    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, Create_ZeroLength_NothingAllocated) {
    uint8_t test_buffer[1] = {0xAA};
    pbbr = pbbr_create(test_buffer, 0);
    ASSERT_EQ(pbbr, nullptr);
}

TEST_F(PartialByteBufferReaderTest, Destroy_NoCrash) {
    uint8_t test_buffer[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);
    pbbr_destroy(&pbbr);
    ASSERT_EQ(pbbr, nullptr);
    SUCCEED();
}

#pragma endregion

#pragma region READ BYTE TESTS

TEST_F(PartialByteBufferReaderTest, ReadByte_OneFullByte_CorrectValueAndCursorPositions) {
    uint8_t test_buffer[2] = {0xAB, 0xCD};
    pbbr = pbbr_create(test_buffer, 2);

    int8_t byte1 = pbbr_read_byte(pbbr, 8);
    ASSERT_EQ(byte1, (int8_t)0xAB);
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 0);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_MultipleFullBytes_CorrectValueAndCursorPositions) {
    uint8_t test_buffer[2] = {0xAB, 0xCD};
    pbbr = pbbr_create(test_buffer, 2);

    int8_t byte1 = pbbr_read_byte(pbbr, 8);
    ASSERT_EQ(byte1, (int8_t)0xAB);
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 0);

    int8_t byte2 = pbbr_read_byte(pbbr, 8);
    ASSERT_EQ(byte2, (int8_t)0xCD);
    ASSERT_EQ(pbbr->byte_pos, 2);
    ASSERT_EQ(pbbr->bit_pos, 0);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_PartialByteWithSignExtension_CorrectValuesAndSign) {
    uint8_t test_buffer[2] = {0b10100000, 0b00000000};
    pbbr = pbbr_create(test_buffer, 2);

    int8_t byte = pbbr_read_byte(pbbr, 3); // Read first 3 bits: 111
    ASSERT_EQ(byte, (int8_t)0xFD); // Sign-extended to -1
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 3);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_PartialByteWithoutSignExtension_CorrectValuesAndSign) {
    uint8_t test_buffer[2] = {0b01100000, 0b00000000};
    pbbr = pbbr_create(test_buffer, 2);

    int8_t byte = pbbr_read_byte(pbbr, 3); // Read first 3 bits: 111
    ASSERT_EQ(byte, (int8_t)0x3); // Sign-extended to -1
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 3);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_PartialBytes_CorrectValuesAndCursorPositions) {
    uint8_t test_buffer[2] = {0b10110101, 0b01111000};
    pbbr = pbbr_create(test_buffer, 2);
    ASSERT_NE(pbbr, nullptr);

    int8_t value1 = pbbr_read_byte(pbbr, 3); // Read first 3 bits: 101
    ASSERT_EQ(value1, (int8_t)0xFD);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 3);

    int8_t value2 = pbbr_read_byte(pbbr, 6); // Read next 6 bits: 101010
    ASSERT_EQ(value2, (int8_t)0xEA);
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 1);

    int8_t value3 = pbbr_read_byte(pbbr, 6); // Read next 6 bits: 111100
    ASSERT_EQ(value3, (int8_t)0xFC);
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 7);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_ExceedBufferLength_ReturnsZeroAndStopsAtEnd) {
    uint8_t test_buffer[1] = {0b10101011};
    pbbr = pbbr_create(test_buffer, 1);

    int8_t value1 = pbbr_read_byte(pbbr, 7); // Read full byte
    ASSERT_EQ(value1, (int8_t)0xD5);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 7);

    int8_t value2 = pbbr_read_byte(pbbr, 2); // Try to read beyond buffer
    ASSERT_EQ(value2, 0);
    ASSERT_EQ(pbbr->byte_pos, 0); // Position should not advance
    ASSERT_EQ(pbbr->bit_pos, 7);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_ZeroBitLength_ReturnsZero) {
    uint8_t test_buffer[1] = {0xFF};
    pbbr = pbbr_create(test_buffer, 1);
    ASSERT_NE(pbbr, nullptr);

    int8_t value1 = pbbr_read_byte(pbbr, 0); // Invalid: 0 bits
    ASSERT_EQ(value1, 0);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 0);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_NullReader_ReturnsZero) {
    int8_t value = pbbr_read_byte(nullptr, 8);
    ASSERT_EQ(value, 0);
}

#pragma endregion

#pragma region READ INT TESTS

TEST_F(PartialByteBufferReaderTest, ReadInt_OneFullInt_CorrectValueAndCursorPositions) {
    uint8_t test_buffer[4] = {0x12, 0x34, 0x56, 0x78};
    pbbr = pbbr_create(test_buffer, 4);

    int value = pbbr_read_int(pbbr, 32);
    ASSERT_EQ(value, (int) 0x12345678);
    ASSERT_EQ(pbbr->byte_pos, 4);
    ASSERT_EQ(pbbr->bit_pos, 0);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_PartialIntsWithSignExtension_CorrectValuesAndSign) {
    uint8_t test_buffer[4] = {0b11100000, 0b00000000};
    pbbr = pbbr_create(test_buffer, 2);

    int value = pbbr_read_int(pbbr, 3); // Read first 3 bits: 111
    ASSERT_EQ(value, -1); // Sign-extended to -1
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 3);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_PartialIntsWithoutSignExtension_CorrectValuesAndSign) {
    uint8_t test_buffer[4] = {0b01100000, 0b00000000};
    pbbr = pbbr_create(test_buffer, 2);

    int value = pbbr_read_int(pbbr, 3); // Read first 3 bits: 111
    ASSERT_EQ(value, 3); // Sign-extended to -1
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 3);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_PartialIntAcrossMultiBytes_CorrectValuesAndCursors) {
    uint8_t test_buffer[3] = {0b10110101, 0b11001100, 0b11110000};
    pbbr = pbbr_create(test_buffer, 3);

    pbbr_read_int(pbbr, 3); // skip the first 3 bits

    int value = pbbr_read_int(pbbr, 15); // Read next 15 bits: 101 0111 0011 0011
    ASSERT_EQ(value, (int)0xFFFFD733); // Sign-extended
    ASSERT_EQ(pbbr->byte_pos, 2);
    ASSERT_EQ(pbbr->bit_pos, 2);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_PartialIntsAlignedEnd_CorrectValuesAndCursorPositions) {
    uint8_t test_buffer[4] = {0b10110101, 0b11001100, 0b10101010, 0b11110000};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);

    int firstInt = pbbr_read_int(pbbr, 9); // Read first 9 bits
    ASSERT_EQ(firstInt, 0xFFFFFF6B); // Sign-extended
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 1);

    int secondInt = pbbr_read_int(pbbr, 13); // Read next 13 bits
    ASSERT_EQ(secondInt, 0xFFFFF32A); // Sign-extended
    ASSERT_EQ(pbbr->byte_pos, 2);
    ASSERT_EQ(pbbr->bit_pos, 6);

    int thirdInt = pbbr_read_int(pbbr, 10); // Read remaining 10 bits
    ASSERT_EQ(thirdInt, 0xFFFFFEF0); // Sign-extended
    ASSERT_EQ(pbbr->byte_pos, 4);
    ASSERT_EQ(pbbr->bit_pos, 0);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_PartialIntsUnalignedEnd_CorrectValuesAndCursorPositions) {
    uint8_t test_buffer[4] = {0b10110101, 0b11001100, 0b10101010, 0b11110000};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);

    int firstInt = pbbr_read_int(pbbr, 9); // Read first 9 bits
    ASSERT_EQ(firstInt, 0xFFFFFF6B);
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 1);

    int secondInt = pbbr_read_int(pbbr, 13); // Read next 13 bits
    ASSERT_EQ(secondInt, 0xFFFFF32A);
    ASSERT_EQ(pbbr->byte_pos, 2);
    ASSERT_EQ(pbbr->bit_pos, 6);

    int thirdInt = pbbr_read_int(pbbr, 3); // Read next 3 bits
    ASSERT_EQ(thirdInt, 0xFFFFFFFD);
    ASSERT_EQ(pbbr->byte_pos, 3);
    ASSERT_EQ(pbbr->bit_pos, 1);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_NegativeValues_CorrectSignExtension) {
    uint8_t test_buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);

    int firstInt = pbbr_read_int(pbbr, 8); // Read 8 bits with high bit set
    ASSERT_EQ(firstInt, -1);
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 0);

    int secondInt = pbbr_read_int(pbbr, 16); // Read 16 bits all ones
    ASSERT_EQ(secondInt, -1);
    ASSERT_EQ(pbbr->byte_pos, 3);
    ASSERT_EQ(pbbr->bit_pos, 0);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_MixedPositiveNegative_CorrectSignExtension) {
    uint8_t test_buffer[2] = {0b01111111, 0b10000000};
    pbbr = pbbr_create(test_buffer, 2);
    ASSERT_NE(pbbr, nullptr);

    int value1 = pbbr_read_int(pbbr, 8); // Positive: 0111 1111
    ASSERT_EQ(value1, 0x7F);
    
    int value2 = pbbr_read_int(pbbr, 8); // Negative: 1000 0000
    ASSERT_EQ(value2, -128);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_ExceedBufferLength_ReturnsZeroAndStopsAtEnd) {
    uint8_t test_buffer[4] = {0xAB, 0xCD, 0xEF, 0x12};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);

    int value1 = pbbr_read_int(pbbr, 31); // Read 31 bits
    ASSERT_EQ(value1, 0xD5E6F789);
    ASSERT_EQ(pbbr->byte_pos, 3);
    ASSERT_EQ(pbbr->bit_pos, 7);

    int value2 = pbbr_read_int(pbbr, 16); // Try to read beyond buffer
    ASSERT_EQ(value2, 0);
    ASSERT_EQ(pbbr->byte_pos, 3); // Position should not advance
    ASSERT_EQ(pbbr->bit_pos, 7);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_ZeroBitLength_ReturnsZero) {
    uint8_t test_buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    pbbr = pbbr_create(test_buffer, 4);

    int value = pbbr_read_int(pbbr, 0); // Invalid: 0 bits
    ASSERT_EQ(value, 0);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 0);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_ExcessiveBitLength_ReturnsZero) {
    uint8_t test_buffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);

    int value = pbbr_read_int(pbbr, 33); // Invalid: more than 32 bits
    ASSERT_EQ(value, 0);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 0);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_NullReader_ReturnsZero) {
    int value = pbbr_read_int(nullptr, 16);
    ASSERT_EQ(value, 0);
}

TEST_F(PartialByteBufferReaderTest, ReadInt_CrossByteBoundaries_CorrectValues) {
    uint8_t test_buffer[4] = {0b11010110, 0b10101100, 0b11110000, 0b00001111};
    pbbr = pbbr_create(test_buffer, 4);

    int value1 = pbbr_read_int(pbbr, 3); // Read 3 bits: 110
    ASSERT_EQ(value1, 0xFFFFFFFE);

    int value2 = pbbr_read_int(pbbr, 17); // Read 17 bits crossing multiple bytes
    ASSERT_EQ(value2, 0xFFFF6ACF);
    ASSERT_EQ(pbbr->byte_pos, 2);
    ASSERT_EQ(pbbr->bit_pos, 4);

    pbbr_destroy(&pbbr);
}

#pragma endregion

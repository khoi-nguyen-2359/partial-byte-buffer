#include <gtest/gtest.h>

#include "partial_byte_buffer_reader.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferReaderTest : public ::testing::Test {
    protected:
        PartialByteBufferReader *pbbr = nullptr;
        void TearDown() override {
            pbbr = nullptr;
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

TEST_F(PartialByteBufferReaderTest, ReadByte_SingleFullByte_CorrectValueAndCursorPositions) {
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

TEST_F(PartialByteBufferReaderTest, ReadByte_PartialBytes_CorrectValuesAndCursorPositions) {
    uint8_t test_buffer[2] = {0b10110101, 0b01111000};
    pbbr = pbbr_create(test_buffer, 2);
    ASSERT_NE(pbbr, nullptr);

    int8_t value1 = pbbr_read_byte(pbbr, 3); // Read first 3 bits: 101
    ASSERT_EQ(value1, (int8_t)0b101);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 3);

    int8_t value2 = pbbr_read_byte(pbbr, 6); // Read next 6 bits: 101010
    ASSERT_EQ(value2, (int8_t)0b101010);
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 1);

    int8_t value3 = pbbr_read_byte(pbbr, 6); // Read next 6 bits: 111100
    ASSERT_EQ(value3, (int8_t)0b111100);
    ASSERT_EQ(pbbr->byte_pos, 1);
    ASSERT_EQ(pbbr->bit_pos, 7);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_ExceedBufferLength_ReturnsZeroAndStopsAtEnd) {
    uint8_t test_buffer[1] = {0xAB};
    pbbr = pbbr_create(test_buffer, 1);
    ASSERT_NE(pbbr, nullptr);

    int8_t value1 = pbbr_read_byte(pbbr, 7); // Read full byte
    ASSERT_EQ(value1, (int8_t)0x55);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 7);

    int8_t value2 = pbbr_read_byte(pbbr, 2); // Try to read beyond buffer
    ASSERT_EQ(value2, 0);
    ASSERT_EQ(pbbr->byte_pos, 0); // Position should not advance
    ASSERT_EQ(pbbr->bit_pos, 7);

    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_ZeroBitLength_ReturnsZero) {
    uint8_t test_buffer[1] = {0xAB};
    pbbr = pbbr_create(test_buffer, 1);
    ASSERT_NE(pbbr, nullptr);

    int8_t value1 = pbbr_read_byte(pbbr, 0); // Invalid: 0 bits
    ASSERT_EQ(value1, 0);
    
    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, ReadByte_NullReader_ReturnsZero) {
    int8_t value = pbbr_read_byte(nullptr, 8);
    ASSERT_EQ(value, 0);
}

#pragma endregion

#pragma region READ INT TESTS
#pragma endregion

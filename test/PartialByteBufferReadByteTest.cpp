#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferReadIntTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(&pbb);
        }
};

TEST_F(PartialByteBufferReadIntTest, ReadByte_FullUnsignedByte_CorrectValueAndCursorPositions) {
    uint8_t data[] = {0xAB, 0xCD};
    pbb = pbb_from_array(data, 2);

    int8_t byte1 = pbb_read_byte(pbb, 8);
    ASSERT_EQ(byte1, (int8_t)0xAB);
    ASSERT_EQ(pbb->read_pos, 8);
}

TEST_F(PartialByteBufferReadIntTest, ReadByte_FullSignedByte_CorrectValueAndCursorPositions) {
    uint8_t data[] = {0b10010110};
    pbb = pbb_from_array(data, 1);

    int8_t byte1 = pbb_read_byte(pbb, 8);
    ASSERT_EQ(byte1, (int8_t)0x96);
    ASSERT_EQ(pbb->read_pos, 8);
}

TEST_F(PartialByteBufferReadIntTest, ReadByte_MultipleFullBytes_CorrectValueAndCursorPositions) {
    uint8_t data[] = {0xAB, 0xCD};
    pbb = pbb_from_array(data, 2);

    int8_t byte1 = pbb_read_byte(pbb, 8);
    ASSERT_EQ(byte1, (int8_t)0xAB);
    ASSERT_EQ(pbb->read_pos, 8);

    int8_t byte2 = pbb_read_byte(pbb, 8);
    ASSERT_EQ(byte2, (int8_t)0xCD);
    ASSERT_EQ(pbb->read_pos, 16);
}

TEST_F(PartialByteBufferReadIntTest, ReadByte_PartialSignedByte_CorrectValuesAndSign) {
    uint8_t data[] = {0b10100000, 0b00000000};
    pbb = pbb_from_array(data, 2);

    int8_t byte = pbb_read_byte(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(byte, (int8_t)0xFD); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadIntTest, ReadByte_PartialUnsignedByte_CorrectValuesAndSign) {
    uint8_t data[] = {0b01100000, 0b00000000};
    pbb = pbb_from_array(data, 2);

    int8_t byte = pbb_read_byte(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(byte, (int8_t)0x3); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadIntTest, ReadByte_PartialBytesUnaligned_CorrectValuesAndCursorPositions) {
    uint8_t data[] = {0b10110101, 0b01111000};
    pbb = pbb_from_array(data, 2);
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

TEST_F(PartialByteBufferReadIntTest, ReadByte_PartialBytesAligned_CorrectValuesAndCursorPositions) {
    uint8_t data[] = {0b10110101, 0b01111000};
    pbb = pbb_from_array(data, 2);
    ASSERT_NE(pbb, nullptr);

    int8_t value1 = pbb_read_byte(pbb, 4); // Read first 4 bits: 1011
    ASSERT_EQ(value1, (int8_t)0xFB);
    ASSERT_EQ(pbb->read_pos, 4);

    int8_t value2 = pbb_read_byte(pbb, 5); // Read next 5 bits: 01010
    ASSERT_EQ(value2, (int8_t)0x0A);
    ASSERT_EQ(pbb->read_pos, 9);

    int8_t value3 = pbb_read_byte(pbb, 7); // Read next 7 bits: 1111000
    ASSERT_EQ(value3, (int8_t)0xF8);
    ASSERT_EQ(pbb->read_pos, 16);
}

TEST_F(PartialByteBufferReadIntTest, ReadByte_ZeroBitLength_ReturnsZero) {
    uint8_t data[] = {0xFF};
    pbb = pbb_from_array(data, 1);
    ASSERT_NE(pbb, nullptr);

    int8_t value1 = pbb_read_byte(pbb, 0); // Invalid: 0 bits
    ASSERT_EQ(value1, 0);
    ASSERT_EQ(pbb->read_pos, 0);
}

TEST_F(PartialByteBufferReadIntTest, ReadByte_NullReader_ReturnsZero) {
    int8_t value = pbb_read_byte(nullptr, 8);
    ASSERT_EQ(value, 0);
}

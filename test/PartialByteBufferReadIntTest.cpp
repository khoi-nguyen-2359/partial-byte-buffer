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

TEST_F(PartialByteBufferReadIntTest, ReadInt_FullUnsignedInt_CorrectValueAndCursorPositions) {
    uint8_t data[] = {0b01111111, 0x34, 0x56, 0x78};
    pbb = pbb_from_array(data, 4);

    int value = pbb_read_int(pbb, 32);
    ASSERT_EQ(value, (int) 0x7F345678);
    ASSERT_EQ(pbb->read_pos, 32);
}

TEST_F(PartialByteBufferReadIntTest, ReadInt_FullSignedInt_CorrectValueAndCursorPositions) {
    uint8_t data[] = {0b11110111, 0x34, 0x56, 0x78};
    pbb = pbb_from_array(data, 4);

    int value = pbb_read_int(pbb, 32);
    ASSERT_EQ(value, (int) 0xF7345678);
    ASSERT_EQ(pbb->read_pos, 32);
}

TEST_F(PartialByteBufferReadIntTest, ReadInt_MultipleFullInts_CorrectValuesAndCursorPositions) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    pbb = pbb_from_array(data, 8);

    int firstValue = pbb_read_int(pbb, 32);
    ASSERT_EQ(firstValue, (int) 0x12345678);
    ASSERT_EQ(pbb->read_pos, 32);

    int secondValue = pbb_read_int(pbb, 32);
    ASSERT_EQ(secondValue, (int) 0x9ABCDEF0);
    ASSERT_EQ(pbb->read_pos, 64);
}

TEST_F(PartialByteBufferReadIntTest, ReadInt_PartialSignedInt_CorrectValuesAndSign) {
    uint8_t data[] = {0b11100000, 0b00000000};
    pbb = pbb_from_array(data, 2);

    int value = pbb_read_int(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(value, -1); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadIntTest, ReadInt_PartialUnsignedInt_CorrectValuesAndSign) {
    uint8_t data[] = {0b01100000, 0b00000000};
    pbb = pbb_from_array(data, 2);

    int value = pbb_read_int(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(value, 3); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadIntTest, ReadInt_PartialIntsAligned_CorrectValuesAndCursorPositions) {
    uint8_t data[] = {0b10110101, 0b11001100, 0b10101010, 0b11110000};
    pbb = pbb_from_array(data, 4);
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

TEST_F(PartialByteBufferReadIntTest, ReadInt_PartialIntsUnaligned_CorrectValuesAndCursorPositions) {
    uint8_t data[] = {0b10110101, 0b11001100, 0b10101010, 0b11110000};
    pbb = pbb_from_array(data, 4);
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

TEST_F(PartialByteBufferReadIntTest, ReadInt_ZeroBitLength_ReturnsZero) {
    uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
    pbb = pbb_from_array(data, 4);

    int value = pbb_read_int(pbb, 0); // Invalid: 0 bits
    ASSERT_EQ(value, 0);
    ASSERT_EQ(pbb->read_pos, 0);
}

TEST_F(PartialByteBufferReadIntTest, ReadInt_NullReader_ReturnsZero) {
    int value = pbb_read_int(nullptr, 16);
    ASSERT_EQ(value, 0);
}

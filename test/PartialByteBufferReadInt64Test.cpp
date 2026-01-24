#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferReadInt64Test : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(&pbb);
        }
};

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_FullUnsignedInt64_CorrectValueAndCursorPositions) {
    uint8_t data[] = {0x01, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    pbb = pbb_from_array(data, 8);

    int64_t value = pbb_read_int64(pbb, 64);
    ASSERT_EQ(value, (int64_t) 0x0122334455667788);
    ASSERT_EQ(pbb->read_pos, 64);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_FullSignedInt64_CorrectValueAndCursorPositions) {
    uint8_t data[] = {0xF1, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    pbb = pbb_from_array(data, 8);

    int64_t value = pbb_read_int64(pbb, 64);
    ASSERT_EQ(value, (int64_t) 0xF122334455667788);
    ASSERT_EQ(pbb->read_pos, 64);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_MultipleFullInt64s_CorrectValuesAndCursorPositions) {
    uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                      0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00};
    pbb = pbb_from_array(data, 16);

    int64_t firstValue = pbb_read_int64(pbb, 64);
    ASSERT_EQ(firstValue, (int64_t) 0x1122334455667788);
    ASSERT_EQ(pbb->read_pos, 64);

    int64_t secondValue = pbb_read_int64(pbb, 64);
    ASSERT_EQ(secondValue, (int64_t) 0x99AABBCCDDEEFF00);
    ASSERT_EQ(pbb->read_pos, 128);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_PartialSignedInt64_CorrectValuesAndSign) {
    uint8_t data[] = {0b11100000, 0b00000000};
    pbb = pbb_from_array(data, 2);

    int64_t value = pbb_read_int64(pbb, 3); // Read first 3 bits: 111
    ASSERT_EQ(value, -1); // Sign-extended to -1
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_PartialUnsignedInt64_CorrectValuesAndSign) {
    uint8_t data[] = {0b01100000, 0b00000000};
    pbb = pbb_from_array(data, 2);

    int64_t value = pbb_read_int64(pbb, 3); // Read first 3 bits: 011
    ASSERT_EQ(value, 3);
    ASSERT_EQ(pbb->read_pos, 3);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_PartialInt64sAligned_CorrectValuesAndCursorPositions) {
    uint8_t data[] = {0b10110101, 0b11001100, 0b10101010, 0b11110000,
                      0b11001100, 0b10101010, 0b11110000, 0b10110101};
    pbb = pbb_from_array(data, 8);
    ASSERT_NE(pbb, nullptr);

    int64_t firstInt = pbb_read_int64(pbb, 17); // Read first 17 bits
    ASSERT_EQ(firstInt, (int64_t) 0xFFFFFFFFFFFF6B99); // Sign-extended
    ASSERT_EQ(pbb->read_pos, 17);

    int64_t secondInt = pbb_read_int64(pbb, 23); // Read next 23 bits
    ASSERT_EQ(secondInt, (int64_t) 0x2AF0CC); // Sign-extended
    ASSERT_EQ(pbb->read_pos, 40);

    int64_t thirdInt = pbb_read_int64(pbb, 24); // Read remaining 24 bits
    ASSERT_EQ(thirdInt, (int64_t) 0xFFFFFFFFFFAAF0B5);
    ASSERT_EQ(pbb->read_pos, 64);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_PartialInt64sUnaligned_CorrectValuesAndCursorPositions) {
    uint8_t data[] = {0b10110101, 0b11001100, 0b10101010, 0b11110000,
                      0b11001100, 0b10101010, 0b11110000, 0b10110101};
    pbb = pbb_from_array(data, 8);
    ASSERT_NE(pbb, nullptr);

    int64_t firstInt = pbb_read_int64(pbb, 17); // Read first 17 bits
    ASSERT_EQ(firstInt, (int64_t) 0xFFFFFFFFFFFF6B99);
    ASSERT_EQ(pbb->read_pos, 17);

    int64_t secondInt = pbb_read_int64(pbb, 23); // Read next 23 bits
    ASSERT_EQ(secondInt, (int64_t) 0x2AF0CC);
    ASSERT_EQ(pbb->read_pos, 40);

    int64_t thirdInt = pbb_read_int64(pbb, 13); // Read next 13 bits
    ASSERT_EQ(thirdInt, (int64_t) 0xFFFFFFFFFFFFF55E);
    ASSERT_EQ(pbb->read_pos, 53);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_ZeroBitLength_ReturnsZero) {
    uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    pbb = pbb_from_array(data, 8);

    int64_t value = pbb_read_int64(pbb, 0); // Invalid: 0 bits
    ASSERT_EQ(value, 0);
    ASSERT_EQ(pbb->read_pos, 0);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_NullReader_ReturnsZero) {
    int64_t value = pbb_read_int64(nullptr, 32);
    ASSERT_EQ(value, 0);
}

TEST_F(PartialByteBufferReadInt64Test, ReadInt64_CrossByteBoundaries_CorrectValues) {
    uint8_t data[] = {0b11010\110, 0b10101100, 0b11110000, 0b00001111,
                      0b11010110, 0b10101100, 0b11110000, 0b00001111};
    pbb = pbb_from_array(data, 8);

    int64_t value1 = pbb_read_int64(pbb, 5); // Read 5 bits: 11010
    ASSERT_EQ(value1, (int64_t) 0xFFFFFFFFFFFFFFFA);

    int64_t value2 = pbb_read_int64(pbb, 37); // Read 37 bits crossing multiple bytes
    ASSERT_EQ(value2, (int64_t) 0xFFFFFFFAB3C03F5A);
    ASSERT_EQ(pbb->read_pos, 42);
}

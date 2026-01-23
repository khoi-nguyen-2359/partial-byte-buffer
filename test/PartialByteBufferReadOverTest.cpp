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

TEST_F(PartialByteBufferReadIntTest, ReadByte_ExceedBufferLength_ReturnsZeroAndStopsAtEnd) {
    uint8_t data[] = {0b10101011};
    pbb = pbb_from_array(data, 1);

    int8_t value1 = pbb_read_byte(pbb, 7);
    ASSERT_EQ(value1, (int8_t)0xD5);
    ASSERT_EQ(pbb->read_pos, 7);

    int8_t value2 = pbb_read_byte(pbb, 2); // Try to read beyond buffer
    ASSERT_EQ(value2, 0);
    ASSERT_EQ(pbb->read_pos, 7); // Position should not advance
}

TEST_F(PartialByteBufferReadIntTest, ReadInt_ExceedsBufferLength_ReturnsZero) {
    uint8_t data[] = {0x12, 0x34};
    pbb = pbb_from_array(data, 2);

    int value1 = pbb_read_int(pbb, 8);
    ASSERT_EQ(value1, 0x12);
    ASSERT_EQ(pbb->read_pos, 8);

    int value2 = pbb_read_int(pbb, 32); // Request 32 bits but only 8 available
    ASSERT_EQ(value2, 0);
    ASSERT_EQ(pbb->read_pos, 8);
}

#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferWriteByteTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(&pbb);
        }
};

TEST_F(PartialByteBufferWriteByteTest, WriteByte_FullByte_CorrectValue) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0x42, 8);
    ASSERT_EQ(pbb->buffer[0], 0x42);
    ASSERT_EQ(pbb->write_pos, 8);
}

TEST_F(PartialByteBufferWriteByteTest, WriteByte_MultipleFullBytes_CorrectValues) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_byte(pbb, 0x22, 8);
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->write_pos, 16);
}

TEST_F(PartialByteBufferWriteByteTest, WriteByte_NullBuffer_DoNothing) {
    partial_byte_buffer* pbb_ptr = nullptr;
    pbb_write_byte(pbb_ptr, 0x42, 8);
    ASSERT_EQ(pbb_ptr, nullptr);
}

TEST_F(PartialByteBufferWriteByteTest, WriteByte_PartialByte_CorrectBufferValues) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->write_pos, 3);
}

TEST_F(PartialByteBufferWriteByteTest, WriteByte_MultiplePartialBytes_CorrectBufferValues) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->write_pos, 3);

    pbb_write_byte(pbb, 0b0100110, 7);
    ASSERT_EQ(pbb->buffer[0], 0b10101001);
    ASSERT_EQ(pbb->buffer[1], 0b10000000);
    ASSERT_EQ(pbb->write_pos, 10);
}

TEST_F(PartialByteBufferWriteByteTest, WriteByte_MixedPartialAndFullBytes_CorrectBufferValues) {
    pbb = pbb_create(2);
    pbb_write_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->write_pos, 3);

    pbb_write_byte(pbb, 0b11111, 5);
    ASSERT_EQ(pbb->buffer[0], 0b10111111);
    ASSERT_EQ(pbb->write_pos, 8);
}

TEST_F(PartialByteBufferWriteByteTest, WriteByte_ManyRandomFullBytes_CorrectBufferContent) {
    const int total_bytes = 100;
    pbb = pbb_create(total_bytes);
    const unsigned int seed = 12345;
    uint8_t expected[total_bytes];
    
    // Generate and write random bytes
    srand(seed);
    for (int i = 0; i < total_bytes; ++i) {
        expected[i] = (uint8_t)(rand() & 0xFF);
        pbb_write_byte(pbb, expected[i], 8);
    }
    
    ASSERT_EQ(pbb->write_pos, total_bytes * 8);
    
    // Verify buffer content
    for (int i = 0; i < total_bytes; ++i) {
        ASSERT_EQ(pbb->buffer[i], expected[i]) << "Mismatch at index " << i;
    }
}

TEST_F(PartialByteBufferWriteByteTest, WriteByte_InvalidBitLength_DoesNothing) {
    pbb = pbb_create(2);

    pbb_write_byte(pbb, 0xAB, 0);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_write_byte(pbb, 0xAB, 9);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);
}

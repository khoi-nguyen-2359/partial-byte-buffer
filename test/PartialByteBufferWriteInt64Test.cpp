#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferWriteInt64Test : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(&pbb);
        }
};

TEST_F(PartialByteBufferWriteInt64Test, WriteInt64_FullInt64_CorrectBufferValues) {
    pbb = pbb_create(8);

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
}

TEST_F(PartialByteBufferWriteInt64Test, WriteInt64_MultipleInt64s_CorrectBufferValuesAndPositions) {
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
}

TEST_F(PartialByteBufferWriteInt64Test, WriteInt64_PartialInt64_Below32Bits_CorrectBufferValues) {
    pbb = pbb_create(3);

    pbb_write_int64(pbb, 0xFFFFFFFFFFABCDEF, 23);

    ASSERT_EQ(pbb->buffer[0], 0x57);
    ASSERT_EQ(pbb->buffer[1], 0x9B);
    ASSERT_EQ(pbb->buffer[2], 0xDE);
    ASSERT_EQ(pbb->write_pos, 23);
}

TEST_F(PartialByteBufferWriteInt64Test, WriteInt64_PartialInt64_Above32Bits_CorrectBufferValues) {
    pbb = pbb_create(6);

    pbb_write_int64(pbb, 0x123456789A, 41);

    ASSERT_EQ(pbb->buffer[0], 0x09);
    ASSERT_EQ(pbb->buffer[1], 0x1A);
    ASSERT_EQ(pbb->buffer[2], 0x2B);
    ASSERT_EQ(pbb->buffer[3], 0x3C);
    ASSERT_EQ(pbb->buffer[4], 0x4D);
    ASSERT_EQ(pbb->buffer[5], 0x00);
    ASSERT_EQ(pbb->write_pos, 41);
}

TEST_F(PartialByteBufferWriteInt64Test, WriteInt64_NullBuffer_DoNothing) {
    partial_byte_buffer* pbb_ptr = nullptr;
    pbb_write_int64(pbb_ptr, 0x123456789ABCDEF0, 64);
    ASSERT_EQ(pbb_ptr, nullptr);
}

TEST_F(PartialByteBufferWriteInt64Test, WriteInt64_TooSmallBitLength_DoesNothing) {
    pbb = pbb_create(8);

    pbb_write_int64(pbb, 0x123456789ABCDEF0, 0);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);
}

TEST_F(PartialByteBufferWriteInt64Test, WriteInt64_TooLargeBitLength_DoesNothing) {
    pbb = pbb_create(8);

    pbb_write_int64(pbb, 0x123456789ABCDEF0, 65);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);
}

TEST_F(PartialByteBufferWriteInt64Test, WriteInt64_ManyRandomPartialInt64s_CorrectBufferContent) {
    const int total_int64s = 100;
    pbb = pbb_create(800);
    
    const unsigned int seed = 12345;
    int64_t expected[total_int64s];
    
    // Generate and write random int64s
    srand(seed);
    for (int i = 0; i < total_int64s; ++i) {
        // Combine two rand() calls to get a full 64-bit value
        expected[i] = ((int64_t)rand() << 32) | ((int64_t)rand() & 0xFFFFFFFF);
        pbb_write_int64(pbb, expected[i], 8);
    }
    
    ASSERT_EQ(pbb->write_pos, total_int64s * 8);
    
    // Verify buffer content
    for (int i = 0; i < total_int64s; ++i) {
        ASSERT_EQ(pbb->buffer[i], expected[i] & 0xFF) << "Mismatch at int64 " << i;
    }
}

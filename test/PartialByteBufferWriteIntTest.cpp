#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferWriteIntTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(&pbb);
        }
};

TEST_F(PartialByteBufferWriteIntTest, WriteInt_FullInt_CorrectBufferValues) {
    pbb = pbb_create(4);

    pbb_write_int(pbb, 0x11223344, 32);

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->write_pos, 32);
    ASSERT_EQ(pbb->capacity, 4);
}

TEST_F(PartialByteBufferWriteIntTest, WriteInt_MultipleFullInts_CorrectBufferValuesAndPositions) {
    pbb = pbb_create(8);

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
}

TEST_F(PartialByteBufferWriteIntTest, WriteInt_PartialInt_CorrectBufferValues) {
    pbb = pbb_create(4);

    pbb_write_int(pbb, 0xABC, 12);

    ASSERT_EQ(pbb->buffer[0], 0xAB);
    ASSERT_EQ(pbb->buffer[1], 0xC0);
    ASSERT_EQ(pbb->write_pos, 12);
}

TEST_F(PartialByteBufferWriteIntTest, WriteInt_MultiplePartialInts_CorrectBufferValues) {
    pbb = pbb_create(2);

    pbb_write_int(pbb, 0xABC, 11);  // 0101 0111 100
    pbb_write_int(pbb, 0xABC, 3);   // 100

    ASSERT_EQ(pbb->buffer[0], 0x57);
    ASSERT_EQ(pbb->buffer[1], 0x90);
    ASSERT_EQ(pbb->write_pos, 14);
}

TEST_F(PartialByteBufferWriteIntTest, WriteInt_ManyRandomPartialInts_CorrectBufferContent) {
    const int total_ints = 100;
    pbb = pbb_create(total_ints);
    
    const unsigned int seed = 12345;
    int expected[total_ints];
    
    // Generate and write random ints
    srand(seed);
    for (int i = 0; i < total_ints; ++i) {
        expected[i] = rand();
        pbb_write_int(pbb, expected[i], 8);
    }

    ASSERT_EQ(pbb->write_pos, total_ints * 8);

    // Verify buffer content
    for (int i = 0; i < total_ints; ++i) {
        ASSERT_EQ(pbb->buffer[i], expected[i] & 0xFF) << "Mismatch at int " << i;
    }
}

TEST_F(PartialByteBufferWriteIntTest, WriteInt_NullBuffer_DoNothing) {
    partial_byte_buffer* pbb_ptr = nullptr;
    pbb_write_int(pbb_ptr, 0x12345678, 32);
    ASSERT_EQ(pbb_ptr, nullptr);
}

TEST_F(PartialByteBufferWriteIntTest, WriteInt_InvalidBitLength_DoesNothing) {
    pbb = pbb_create(2);
    
    pbb_write_int(pbb, 0x12345678, 0);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);

    pbb_write_int(pbb, 0x12345678, 33);
    ASSERT_EQ(pbb->buffer[0], 0);
    ASSERT_EQ(pbb->write_pos, 0);
    ASSERT_EQ(pbb->capacity, 2);
}

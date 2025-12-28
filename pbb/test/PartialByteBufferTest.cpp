#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferTest : public ::testing::Test {
    protected:
        PartialByteBuffer *pbb = nullptr;
        void TearDown() override {
            pbb = nullptr;
        }
};

#pragma region CREATE - DESTROY TESTS

TEST_F(PartialByteBufferTest, Create_CorrectAllocation_CorrectCursors) {
    pbb = pbb_create(10);
    ASSERT_NE(pbb, nullptr);
    ASSERT_EQ(pbb->capacity, 16);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(pbb);
}

TEST_F(PartialByteBufferTest, Create_ZeroCapacity_NothingAllocated) {
    pbb = pbb_create(0);
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferTest, Create_NegativeCapacity_NothingAllocated) {
    pbb = pbb_create(-1);
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferTest, Destroy_NoCrash) {
    pbb = pbb_create(1);
    ASSERT_NE(pbb, nullptr);
    pbb_destroy(pbb);
    SUCCEED();
}

#pragma endregion

#pragma region PUT BYTE TESTS

TEST_F(PartialByteBufferTest, PutByte_SingleByte_CorrectValue) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0x42, 8);
    ASSERT_EQ(pbb->buffer[0], 0x42);
    ASSERT_EQ(pbb->byte_pos, 1);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(pbb);
}

TEST_F(PartialByteBufferTest, PutByte_MultipleBytes_CorrectValues) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0x11, 8);
    pbb_put_byte(pbb, 0x22, 8);
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->byte_pos, 2);
    ASSERT_EQ(pbb->bit_pos, 0);
    pbb_destroy(pbb);
}

TEST_F(PartialByteBufferTest, PutByte_ExceedCapacity_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0x11, 8);
    pbb_put_byte(pbb, 0x22, 8);
    pbb_put_byte(pbb, 0x33, 8);  // This should exceed capacity
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->byte_pos, 3);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_put_byte(pbb, 0x44, 8);
    pbb_put_byte(pbb, 0x55, 8);  // This should exceed capacity again
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->buffer[4], 0x55);
    ASSERT_EQ(pbb->byte_pos, 5);
    ASSERT_EQ(pbb->bit_pos, 0);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(pbb);
}

TEST_F(PartialByteBufferTest, PutByte_NullBuffer_DoNothing) {
    PartialByteBuffer* pbb_ptr = nullptr;
    pbb_put_byte(pbb_ptr, 0x42, 8);
    ASSERT_EQ(pbb_ptr, nullptr);
}

TEST_F(PartialByteBufferTest, PutByte_PartialByte_3Bits_CorrectValue) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_put_byte(pbb, 0b101, 3);
    ASSERT_EQ(pbb->buffer[0], 0b10100000);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 3);
    pbb_destroy(pbb);
}

// TEST_F(PartialByteBufferTest, PutByte_PartialByte_4Bits_CorrectValue) {
//     pbb = pbb_create(2);
//     ASSERT_NE(pbb, nullptr);
//     // Put 4 bits with value 0b1010 (10)
//     pbb_put_byte(pbb, 0b1010, 4);
//     ASSERT_EQ(pbb->buffer[0] & 0b11110000, 0b10100000);
//     ASSERT_EQ(pbb->byte_pos, 0);
//     ASSERT_EQ(pbb->bit_pos, 4);
//     pbb_destroy(pbb);
// }

// TEST_F(PartialByteBufferTest, PutByte_TwoPartialBytes_SameByte_CorrectValue) {
//     pbb = pbb_create(2);
//     ASSERT_NE(pbb, nullptr);
//     // Put 3 bits: 0b101
//     pbb_put_byte(pbb, 0b101, 3);
//     // Put 5 bits: 0b11001
//     pbb_put_byte(pbb, 0b11001, 5);
//     // Combined: 0b10111001
//     ASSERT_EQ(pbb->buffer[0], (int8_t)0b10111001);
//     ASSERT_EQ(pbb->byte_pos, 1);
//     ASSERT_EQ(pbb->bit_pos, 0);
//     pbb_destroy(pbb);
// }

// TEST_F(PartialByteBufferTest, PutByte_PartialBytes_SpanTwoBytes_CorrectValue) {
//     pbb = pbb_create(2);
//     ASSERT_NE(pbb, nullptr);
//     // Put 5 bits: 0b10101
//     pbb_put_byte(pbb, 0b10101, 5);
//     // Put 6 bits: 0b110011
//     pbb_put_byte(pbb, 0b110011, 6);
//     // First byte: 0b10101110 (5 + 3 bits)
//     // Second byte: 0b01100000 (remaining 3 bits)
//     ASSERT_EQ(pbb->buffer[0], (int8_t)0b10101110);
//     ASSERT_EQ(pbb->buffer[1] & 0b11100000, 0b01100000);
//     ASSERT_EQ(pbb->byte_pos, 1);
//     ASSERT_EQ(pbb->bit_pos, 3);
//     pbb_destroy(pbb);
// }

// TEST_F(PartialByteBufferTest, PutByte_MultipleSingleBits_CorrectValue) {
//     pbb = pbb_create(2);
//     ASSERT_NE(pbb, nullptr);
//     // Put individual bits: 1, 0, 1, 1, 0, 0, 1, 0
//     pbb_put_byte(pbb, 1, 1);
//     pbb_put_byte(pbb, 0, 1);
//     pbb_put_byte(pbb, 1, 1);
//     pbb_put_byte(pbb, 1, 1);
//     pbb_put_byte(pbb, 0, 1);
//     pbb_put_byte(pbb, 0, 1);
//     pbb_put_byte(pbb, 1, 1);
//     pbb_put_byte(pbb, 0, 1);
//     // Result: 0b10110010
//     ASSERT_EQ(pbb->buffer[0], (int8_t)0b10110010);
//     ASSERT_EQ(pbb->byte_pos, 1);
//     ASSERT_EQ(pbb->bit_pos, 0);
//     pbb_destroy(pbb);
// }

// TEST_F(PartialByteBufferTest, PutByte_PartialThenFullByte_CorrectValue) {
//     pbb = pbb_create(2);
//     ASSERT_NE(pbb, nullptr);
//     // Put 4 bits: 0b1010
//     pbb_put_byte(pbb, 0b1010, 4);
//     // Put full 8 bits: 0b11110000
//     pbb_put_byte(pbb, 0b11110000, 8);
//     // First byte: 0b10101111 (4 + 4 bits)
//     // Second byte: 0b00000000 (remaining 4 bits)
//     ASSERT_EQ(pbb->buffer[0], (int8_t)0b10101111);
//     ASSERT_EQ(pbb->buffer[1] & 0b11110000, 0b00000000);
//     ASSERT_EQ(pbb->byte_pos, 1);
//     ASSERT_EQ(pbb->bit_pos, 4);
//     pbb_destroy(pbb);
// }



#pragma endregion
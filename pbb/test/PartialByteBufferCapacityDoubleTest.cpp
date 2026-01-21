#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferCapacityDoubleTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            if (pbb != nullptr) {
                pbb_destroy(&pbb);
            }
        }
};

TEST_F(PartialByteBufferCapacityDoubleTest, WriteByte_ExceedCapacityOnce_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_byte(pbb, 0x22, 8);
    pbb_write_byte(pbb, 0x33, 8);  // This should exceed capacity
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->write_pos, 24);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferCapacityDoubleTest, WriteByte_ExceedCapacityTwice_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);
    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_byte(pbb, 0x22, 8);
    pbb_write_byte(pbb, 0x33, 8);  // This should exceed capacity
    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->write_pos, 24);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_write_byte(pbb, 0x44, 8);
    pbb_write_byte(pbb, 0x55, 8);  // This should exceed capacity again
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->buffer[4], 0x55);
    ASSERT_EQ(pbb->write_pos, 40);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}


TEST_F(PartialByteBufferCapacityDoubleTest, WriteInt_ExceedCapacityOnce_BufferGrowsCorrectly) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_int(pbb, 0x11223344, 32); // requires growth past initial 2 bytes

    ASSERT_EQ(pbb->buffer[0], 0x11);
    ASSERT_EQ(pbb->buffer[1], 0x22);
    ASSERT_EQ(pbb->buffer[2], 0x33);
    ASSERT_EQ(pbb->buffer[3], 0x44);
    ASSERT_EQ(pbb->capacity, 4);
    ASSERT_EQ(pbb->write_pos, 32);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferCapacityDoubleTest, PutPartialByteThenPartialInt_ExceedCapacity_CorrectBufferValuesAndCapacity) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0b10100, 5);      
    pbb_write_int(pbb, 0b101010111101, 12);

    ASSERT_EQ(pbb->buffer[0], 0b10100101);
    ASSERT_EQ(pbb->buffer[1], 0b01011110);
    ASSERT_EQ(pbb->buffer[2], 0b10000000);
    ASSERT_EQ(pbb->write_pos, 17);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}


TEST_F(PartialByteBufferCapacityDoubleTest, WriteByteThenInt_ExceedCapacityTwiceAtOnce_CorrectBufferCapacity) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0x11, 8);
    pbb_write_int(pbb, 0x22334455, 32); // Exceeds initial capacity from 2 -> 5 * 2
    ASSERT_EQ(pbb->capacity, 10);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferCapacityDoubleTest, WriteByte_ExactlyAtCapacity_NoGrowthYet) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0xAA, 8);
    pbb_write_byte(pbb, 0xBB, 8);

    ASSERT_EQ(pbb->buffer[0], 0xAA);
    ASSERT_EQ(pbb->buffer[1], 0xBB);
    ASSERT_EQ(pbb->write_pos, 16);
    ASSERT_EQ(pbb->capacity, 2);  // No growth should happen yet

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferCapacityDoubleTest, WritePartialByte_AtMaxCap_GrowsAndPreservesData) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0xFF, 8);          // Byte 0 full
    pbb_write_byte(pbb, 0xF0, 8);          // Byte 1 full, at capacity
    pbb_write_byte(pbb, 0b1010, 4);        // Partial byte, triggers growth

    ASSERT_EQ(pbb->buffer[0], 0xFF);
    ASSERT_EQ(pbb->buffer[1], 0xF0);
    ASSERT_EQ(pbb->buffer[2], 0xA0);
    ASSERT_EQ(pbb->write_pos, 20);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferCapacityDoubleTest, WriteInt_PartialBytesWithinCap_CapMaintained) {
    pbb = pbb_create(4);
    ASSERT_NE(pbb, nullptr);

    // Write 5 bits first
    pbb_write_byte(pbb, 0b10101, 5);
    // Write 24-bit int that still doesn't cause growth and span multiple bytes
    pbb_write_int(pbb, 0xABCDEF, 24);

    ASSERT_EQ(pbb->buffer[0], 0b10101101);
    ASSERT_EQ(pbb->buffer[1], 0b01011110);
    ASSERT_EQ(pbb->buffer[2], 0b01101111);
    ASSERT_EQ(pbb->buffer[3], 0b01111000);

    ASSERT_EQ(pbb->write_pos, 29);
    ASSERT_EQ(pbb->capacity, 4);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferCapacityDoubleTest, WriteFullBytes_WriteMultipleTimes_CorrectFinalCapacity) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    // Write 16 bytes of data - will require multiple growth cycles
    for (int i = 0; i < 16; i++) {
        pbb_write_byte(pbb, (uint8_t)(i + 1), 8);
    }

    // Verify all data
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(pbb->buffer[i], (uint8_t)(i + 1));
    }
    ASSERT_EQ(pbb->write_pos, 128);
    ASSERT_EQ(pbb->capacity, 16);  // 2 -> 4 -> 8 -> 16

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferCapacityDoubleTest, WritePartialBytes_WriteMultipleTimes_CorrectFinalCapacity) {
    int cap = 2;
    pbb = pbb_create(cap);
    ASSERT_NE(pbb, nullptr);

    const int random_seed = 42;
    srand(random_seed);
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        uint8_t bits = abs(rand()) % 8 + 1;
        sum += bits;
        if (sum > cap * 8) {
            cap *= 2;
        }
        pbb_write_byte(pbb, (uint8_t)(i + 1), bits);
    }

    ASSERT_EQ(pbb->capacity, cap);

    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferCapacityDoubleTest, AlternatingBytesAndInts_MultipleGrowths_DataIntegrity) {
    pbb = pbb_create(2);
    ASSERT_NE(pbb, nullptr);

    pbb_write_byte(pbb, 0xAA, 8);         // 1 byte, capacity 2
    pbb_write_int(pbb, 0xBBCC, 16);       // 3 bytes total, capacity 4
    pbb_write_byte(pbb, 0xDD, 8);         // 4 bytes total, capacity 4
    pbb_write_int(pbb, 0xEEFF1122, 32);   // 8 bytes total, capacity 8

    ASSERT_EQ(pbb->buffer[0], 0xAA);
    ASSERT_EQ(pbb->buffer[1], 0xBB);
    ASSERT_EQ(pbb->buffer[2], 0xCC);
    ASSERT_EQ(pbb->buffer[3], 0xDD);
    ASSERT_EQ(pbb->buffer[4], 0xEE);
    ASSERT_EQ(pbb->buffer[5], 0xFF);
    ASSERT_EQ(pbb->buffer[6], 0x11);
    ASSERT_EQ(pbb->buffer[7], 0x22);
    ASSERT_EQ(pbb->write_pos, 64);
    ASSERT_EQ(pbb->capacity, 8);

    pbb_destroy(&pbb);
}


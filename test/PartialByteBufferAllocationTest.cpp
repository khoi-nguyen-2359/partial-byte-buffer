#include <gtest/gtest.h>

#include "partial_byte_buffer.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferAllocationTest : public ::testing::Test {
    protected:
        partial_byte_buffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(&pbb);
        }
};

#pragma region CREATE - DESTROY TESTS

TEST_F(PartialByteBufferAllocationTest, Create_CorrectAllocation_CorrectCursors) {
    pbb = pbb_create(10);
    ASSERT_NE(pbb, nullptr);
    ASSERT_EQ(pbb->capacity, 10);
    ASSERT_EQ(pbb->write_pos, 0);
    pbb_destroy(&pbb);
}

TEST_F(PartialByteBufferAllocationTest, Create_ZeroCapacity_NothingAllocated) {
    pbb = pbb_create(0);
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferAllocationTest, Create_NegativeCapacity_NothingAllocated) {
    pbb = pbb_create(-1);
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferAllocationTest, Destroy_NoCrash) {
    pbb = pbb_create(1);
    ASSERT_NE(pbb, nullptr);
    pbb_destroy(&pbb);
    SUCCEED();
}

TEST_F(PartialByteBufferAllocationTest, Destroy_NullPointer_NoCrash) {
    pbb = nullptr;
    pbb_destroy(&pbb);
    ASSERT_EQ(pbb, nullptr);
    SUCCEED();
}

#pragma endregion

#pragma region CREATE FROM ARRAY TESTS

TEST_F(PartialByteBufferAllocationTest, FromArray_FewElements_CorrectBufferAndCursors) {
    uint8_t array[] = {0x12, 0x34, 0x56, 0x78};
    pbb = pbb_from_array(array, 4);
    
    ASSERT_NE(pbb, nullptr);
    ASSERT_EQ(pbb->capacity, 4);
    ASSERT_EQ(pbb->write_pos, 32);
    ASSERT_EQ(pbb->read_pos, 0);
    
    ASSERT_EQ(pbb->buffer[0], 0x12);
    ASSERT_EQ(pbb->buffer[1], 0x34);
    ASSERT_EQ(pbb->buffer[2], 0x56);
    ASSERT_EQ(pbb->buffer[3], 0x78);
}

TEST_F(PartialByteBufferAllocationTest, FromArray_NonNullArrayZeroSize_NothingAllocated) {
    uint8_t array[] = {0x12, 0x34, 0x56};
    pbb = pbb_from_array(array, 0);
    
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferAllocationTest, FromArray_NullArray_NothingAllocated) {
    pbb = pbb_from_array(nullptr, 10);
    
    ASSERT_EQ(pbb, nullptr);
}

TEST_F(PartialByteBufferAllocationTest, FromArray_ManyRandomElements_CorrectBufferContent) {
    const int total_bytes = 100;
    uint8_t array[total_bytes];
    const unsigned int seed = 12345;
    
    // Generate random bytes
    srand(seed);
    for (int i = 0; i < total_bytes; ++i) {
        array[i] = (uint8_t)(rand() & 0xFF);
    }
    
    pbb = pbb_from_array(array, total_bytes);
    
    ASSERT_NE(pbb, nullptr);
    ASSERT_EQ(pbb->capacity, total_bytes);
    ASSERT_EQ(pbb->write_pos, total_bytes * 8);
    ASSERT_EQ(pbb->read_pos, 0);
    
    // Verify buffer content matches input array
    for (int i = 0; i < total_bytes; ++i) {
        ASSERT_EQ(pbb->buffer[i], array[i]) << "Mismatch at index " << i;
    }
}

#pragma endregion

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
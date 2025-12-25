#include <gtest/gtest.h>

#include "partial_byte_buffer.h"

class PartialByteBufferTest : public ::testing::Test {
    protected:
        PartialByteBuffer *pbb = nullptr;
        void TearDown() override {
            pbb = nullptr;
            // pbb_destroy(pbb);
        }
};

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
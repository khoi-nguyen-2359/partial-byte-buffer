#include <gtest/gtest.h>

#include "partial_byte_buffer.h"

class PartialByteBufferTest : public ::testing::Test {
    protected:
        PartialByteBuffer *pbb = nullptr;
        void TearDown() override {
            pbb_destroy(pbb);
        }
};

TEST_F(PartialByteBufferTest, Create_CorrectAllocation) {
    pbb = pbb_create(10);
    ASSERT_NE(pbb, nullptr);
    ASSERT_GE(pbb->capacity, 10);
    ASSERT_EQ(pbb->byte_pos, 0);
    ASSERT_EQ(pbb->bit_pos, 0);
}
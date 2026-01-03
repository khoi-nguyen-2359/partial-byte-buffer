#include <gtest/gtest.h>

#include "partial_byte_buffer_reader.h"
#include <stddef.h>
#include <stdint.h>

class PartialByteBufferReaderTest : public ::testing::Test {
    protected:
        PartialByteBufferReader *pbbr = nullptr;
        void TearDown() override {
            pbbr = nullptr;
        }
};

#pragma region CREATE - DESTROY TESTS

TEST_F(PartialByteBufferReaderTest, Create_CorrectAllocation_CorrectCursors) {
    uint8_t test_buffer[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);
    ASSERT_EQ(pbbr->length, 4);
    ASSERT_EQ(pbbr->byte_pos, 0);
    ASSERT_EQ(pbbr->bit_pos, 0);
    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, Create_ZeroLength_NothingAllocated) {
    uint8_t test_buffer[1] = {0xAA};
    pbbr = pbbr_create(test_buffer, 0);
    ASSERT_NE(pbbr, nullptr);
    ASSERT_EQ(pbbr->length, 0);
    pbbr_destroy(&pbbr);
}

TEST_F(PartialByteBufferReaderTest, Destroy_NoCrash) {
    uint8_t test_buffer[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    pbbr = pbbr_create(test_buffer, 4);
    ASSERT_NE(pbbr, nullptr);
    pbbr_destroy(&pbbr);
    SUCCEED();
}

#pragma endregion
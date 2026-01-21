#include "partial_byte_buffer.h"
#include <gtest/gtest.h>
#include <stdint.h>
#include <string.h>
#include <bit>

class FloatResizerTest : public ::testing::Test {
};

TEST_F(FloatResizerTest, ResizeFloat_CompressAndRestore_ReturnsSameValue) {
    double original = 3.141592653589793;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 4;
    int dst_mant_bits = 31;
    double multiplier = 1e9;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
    qword wq;
    wq.uint64_val = restored;

    ASSERT_EQ((int) std::round(original * multiplier), (int) std::round(wq.double_val * multiplier));
}

TEST_F(FloatResizerTest, ResizeFloat_SmallPositiveValue_ReturnsSameValue) {
    double original = 0.123456789;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 18;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);

    ASSERT_EQ(0x2FE6B7, resized);
}

TEST_F(FloatResizerTest, ResizeFloat_LargeNegativeValue_ReturnsSameValue) {
    double original = -12345.6789;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 7;
    int dst_mant_bits = 20;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    // printf("Resized value: 0x%lX\n", resized);
    ASSERT_EQ(0xCC81CD6, resized);
}

TEST_F(FloatResizerTest, ResizeFloat_Zero_ReturnsSameValue) {
    double original = 0.0;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 4;
    int dst_mant_bits = 10;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
    qword wq;
    wq.uint64_val = restored;

    ASSERT_DOUBLE_EQ(original, wq.double_val);
}

TEST_F(FloatResizerTest, ResizeFloat_NegativeZero_ReturnsSameValue) {
    double original = -0.0;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 4;
    int dst_mant_bits = 10;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
    qword wq;
    wq.uint64_val = restored;

    ASSERT_DOUBLE_EQ(original, wq.double_val);
}

TEST_F(FloatResizerTest, ResizeFloat_VerySmallValue_ReturnsSameValue) {
    double original = 1e-8;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 6;
    int dst_mant_bits = 15;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    // printf("Resized value: 0x%lX\n", resized);
    ASSERT_EQ(0x22BCC, resized);
}

TEST_F(FloatResizerTest, ResizeFloat_AggressiveCompression_ReturnsSimilarValue) {
    double original = 42.195;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 4;
    int dst_mant_bits = 8;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);

    ASSERT_EQ(0xC51, resized);
}

TEST_F(FloatResizerTest, ResizeFloat_One_ReturnsSameValue) {
    double original = 1.0;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 15;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
    qword wq;
    wq.uint64_val = restored;

    ASSERT_DOUBLE_EQ(original, wq.double_val);
}

TEST_F(FloatResizerTest, ResizeFloat_NegativeOne_ReturnsSameValue) {
    double original = -1.0;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 15;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
    qword wq;
    wq.uint64_val = restored;

    ASSERT_DOUBLE_EQ(original, wq.double_val);
}

TEST_F(FloatResizerTest, ResizeFloat_PositiveInfinity_ReturnsSameValue) {
    double original = INFINITY;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 15;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
    qword wq;
    wq.uint64_val = restored;

    ASSERT_TRUE(std::isinf(wq.double_val) && wq.double_val > 0);
}

TEST_F(FloatResizerTest, ResizeFloat_NegativeInfinity_ReturnsSameValue) {
    double original = -INFINITY;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 15;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
    qword wq;
    wq.uint64_val = restored;

    ASSERT_TRUE(std::isinf(wq.double_val) && wq.double_val < 0);
}

TEST_F(FloatResizerTest, ResizeFloat_SmallestPositiveSubnormal_ReturnsSimilarValue) {
    double original = 4.9406564584124654e-324;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 3;
    int dst_mant_bits = 52;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
    uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
    qword wq;
    wq.uint64_val = restored;

    ASSERT_EQ(original, wq.double_val);
}

TEST_F(FloatResizerTest, ResizeFloat_SmallestSubnormal_ReturnsSimilarValue) {
    double original = -4.9406564584124654e-324;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 15;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);

    ASSERT_EQ(0x100000, resized);
}

TEST_F(FloatResizerTest, ResizeFloat_LargestSubnormal_ReturnsLargestSubnormal) {
    double original = 2.225073858507201e-308;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 20;

    uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);

    ASSERT_EQ(0xFFFFF, resized);
}

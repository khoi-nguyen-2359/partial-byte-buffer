#include "float_resizer.h"
#include <gtest/gtest.h>
#include <stdint.h>
#include <string.h>
#include <bit>

class FloatResizerTest : public ::testing::Test {
};

TEST_F(FloatResizerTest, ResizeFloat_CompressAndRestore_LongitudeRange_RestoreEquivalentValues) {
    for (int i = 0; i <= 180; ++i) {
        for (int j = 0; j <= 999999; ++j) {
            for (int k = -1; k <= 1; k += 2) {
                double original = k * (i + j * 1e-6);  // Values from -180.999999 to 180.999999
                int src_exp_bits = 11;
                int src_mant_bits = 52;
                int dst_exp_bits = 6;
                int dst_mant_bits = 27;

                uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
                uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
                qword wq;
                wq.uint64_val = restored;

                EXPECT_NEAR(original, wq.double_val, 1e-6);
            }
        }
    }
}

TEST_F(FloatResizerTest, ResizeFloat_CompressAndRestore_LatitudeRange_RestoreEquivalentValues) {
    for (int i = 0; i <= 90; ++i) {
        for (int j = 0; j <= 999999; ++j) {
            for (int k = -1; k <= 1; k += 2) {
                double original = k * (i + j * 1e-6);  // Values from -90.999999 to 90.999999
                int src_exp_bits = 11;
                int src_mant_bits = 52;
                int dst_exp_bits = 6;
                int dst_mant_bits = 26;

                uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
                uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
                qword wq;
                wq.uint64_val = restored;

                EXPECT_NEAR(original, wq.double_val, 1e-6);
            }
        }
    }
}


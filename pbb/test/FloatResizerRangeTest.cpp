#include "float_resizer.h"
#include <gtest/gtest.h>
#include <stdint.h>
#include <string.h>
#include <bit>

class FloatResizerTest : public ::testing::Test {
};

TEST_F(FloatResizerTest, ResizeFloat_LongitudeRange_RestoreEquivalentValues) {
    for (int i = 0; i <= 180; ++i) {
        int maxj = (i == 180) ? 0 : 99999;
        double tolerance = 1e-5;
        for (int j = 0; j <= maxj; ++j) {
            for (int k = -1; k <= 1; k += 2) {
                double original = k * (i + j * tolerance);  // Values from -180.00000 to 180.00000
                int src_exp_bits = 11;
                int src_mant_bits = 52;
                int dst_exp_bits = 6;
                int dst_mant_bits = 24;

                uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
                uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
                qword wq;
                wq.uint64_val = restored;

                EXPECT_NEAR(original, wq.double_val, tolerance);
            }
        }
    }
}

TEST_F(FloatResizerTest, ResizeFloat_LatitudeRange_RestoreEquivalentValues) {
    for (int i = 0; i <= 90; ++i) {
        int maxj = (i == 180) ? 0 : 99999;
        double tolerance = 1e-5;
        for (int j = 0; j <= maxj; ++j) {
            for (int k = -1; k <= 1; k += 2) {
                double original = k * (i + j * tolerance);  // Values from -90.00000 to 90.00000
                // printf("Testing latitude: %f\n", original);
                int src_exp_bits = 11;
                int src_mant_bits = 52;
                int dst_exp_bits = 6;
                int dst_mant_bits = 23;

                uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
                uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
                qword wq;
                wq.uint64_val = restored;

                EXPECT_NEAR(original, wq.double_val, tolerance);
            }
        }
    }
}

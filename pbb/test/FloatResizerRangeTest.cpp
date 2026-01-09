#include "float_resizer.h"
#include <gtest/gtest.h>
#include <stdint.h>
#include <string.h>
#include <bit>

class FloatResizerTest : public ::testing::Test {
};

TEST_F(FloatResizerTest, ResizeFloat_LongitudeRange_RestoreEquivalentValues) {
    double precision = 1e-5;
    double abs_error = 1e-6;
    for (int integral = 0; integral <= 180; ++integral) {
        int fractional = (integral == 180) ? 0 : 99999;
        for (int j = 0; j <= fractional; ++j) {
            for (int sign = -1; sign <= 1; sign += 2) {
                double original = sign * (integral + j * precision);  // Values from -180.00000 to 180.00000
                int src_exp_bits = 11;
                int src_mant_bits = 52;
                int dst_exp_bits = 6;
                int dst_mant_bits = 27;

                uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
                uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
                qword wq;
                wq.uint64_val = restored;

                EXPECT_NEAR(original, wq.double_val, abs_error);
            }
        }
    }
}

TEST_F(FloatResizerTest, ResizeFloat_LatitudeRange_RestoreEquivalentValues) {
    double precision = 1e-5;
    double abs_error = 1e-6;
    for (int integral = 0; integral <= 90; ++integral) {
        int fractional = (integral == 180) ? 0 : 99999;
        for (int j = 0; j <= fractional; ++j) {
            for (int sign = -1; sign <= 1; sign += 2) {
                double original = sign * (integral + j * precision);  // Values from -90.00000 to 90.00000
                // printf("Testing latitude: %f\n", original);
                int src_exp_bits = 11;
                int src_mant_bits = 52;
                int dst_exp_bits = 6;
                int dst_mant_bits = 23;

                uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
                uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
                qword wq;
                wq.uint64_val = restored;

                EXPECT_NEAR(original, wq.double_val, abs_error);
            }
        }
    }
}

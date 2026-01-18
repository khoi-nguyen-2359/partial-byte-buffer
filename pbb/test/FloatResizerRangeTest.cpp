#include "float_resizer.h"
#include <gtest/gtest.h>
#include <stdint.h>
#include <string.h>
#include <bit>
#include <cmath>

static int roundToDecimalPlaces(double value, double multiplier) {
    return (int) std::round(value * multiplier);
}

#define EXPECT_NEAR_ROUNDED(val1, val2, multiplier) \
    EXPECT_EQ((int) std::round(val1 * multiplier), (int) std::round(val2 * multiplier))

class FloatResizerTest : public ::testing::Test {
};

/**
 * Longitude Range Test
 * ---------------------
 * Range:      -180.00000 to +180.00000 degrees
 * Precision:  0.00001 degrees (5 decimal places, ~1.1 meters at equator)
 * Total:      36,000,001 test values
 * Format:     6 exponent bits + 25 mantissa bits (32 bits total)
 */
TEST_F(FloatResizerTest, ResizeFloat_LongitudeRange_RestoreEquivalentValues) {
    double precision = 1e-5;
    double multiplier = 1e5;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 6;
    int dst_mant_bits = 25;

    for (int i = -180 * multiplier; i <= 180 * multiplier; ++i) {
        double original = i * precision;  // Values from -180.00000 to 180.00000

        uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
        uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
        qword wq;
        wq.uint64_val = restored;

        EXPECT_EQ((int) std::round(original * multiplier), (int) std::round(wq.double_val * multiplier));
    }
}

/**
 * Latitude Range Test
 * --------------------
 * Range:      -90.00000 to +90.00000 degrees
 * Precision:  0.00001 degrees (5 decimal places, ~1.1 meters)
 * Total:      18,000,001 test values
 * Format:     6 exponent bits + 24 mantissa bits (31 bits total)
 */
TEST_F(FloatResizerTest, ResizeFloat_LatitudeRange_RestoreEquivalentValues) {
    double precision = 1e-5;
    double multiplier = 1e5;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 6;
    int dst_mant_bits = 24;

    for (int i = -9000000; i <= 9000000; ++i) {
        double original = i * precision;  // Values from -90.00000 to 90.00000

        uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
        uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
        qword wq;
        wq.uint64_val = restored;

        EXPECT_EQ((int) std::round(original * multiplier), (int) std::round(wq.double_val * multiplier));
    }
}

/**
 * Altitude/Elevation Range Test (Below Sea Level)
 * ------------------------------------------------
 * Range:      -500.00m (Dead Sea depression) to 0.00m (sea level)
 * Precision:  0.01m (centimeter)
 * Total:      50,001 test values
 * Format:     5 exponent bits + 16 mantissa bits (22 bits total)
 */
TEST_F(FloatResizerTest, ResizeFloat_AltitudeRangeBelowSeaLevel_RestoreEquivalentValues) {
    double precision = 0.01;
    double multiplier = 100.0;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 16;

    for (int i = -500 * multiplier; i <= 0; ++i) {
        double original = i * precision;  // Values from -500.00 to 0.00

        uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
        uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
        qword wq;
        wq.uint64_val = restored;

        EXPECT_EQ((int) std::round(original * multiplier), (int) std::round(wq.double_val * multiplier));
    }
}

/**
 * Altitude/Elevation Range Test (Above Sea Level)
 * ------------------------------------------------
 * Range:      0.00m (sea level) to +15000.00m (commercial aircraft operating altitude)
 * Precision:  0.01m (centimeter)
 * Total:      1,500,001 test values
 * Format:     5 exponent bits + 21 mantissa bits (27 bits total)
 */
TEST_F(FloatResizerTest, ResizeFloat_AltitudeRangeAboveSeaLevel_RestoreEquivalentValues) {
    double precision = 0.01;
    double multiplier = 100.0;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 21;

    for (int i = 0; i <= 15000 * multiplier; ++i) {
        double original = i * precision;  // Values from 0.00 to 15000.00

        uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
        uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
        qword wq;
        wq.uint64_val = restored;

        EXPECT_EQ((int) std::round(original * multiplier), (int) std::round(wq.double_val * multiplier));
    }
}

/**
 * Temperature Range Test
 * ----------------------
 * Range:      -273.15°C (absolute zero) to +1000.00°C (molten lava)
 * Precision:  0.01°C (centidegree)
 * Total:      127,316 test values
 * Format:     5 exponent bits + 17 mantissa bits (23 bits total)
 */
TEST_F(FloatResizerTest, ResizeFloat_TemperatureRange_RestoreEquivalentValues) {
    double precision = 0.01;
    double multiplier = 100.0;
    int src_exp_bits = 11;
    int src_mant_bits = 52;
    int dst_exp_bits = 5;
    int dst_mant_bits = 17;

    // Test negative range: -273.15 to 1000.00 (scaled by 100)
    for (int i = -273.15 * multiplier; i <= 1000 * multiplier; ++i) {
        double original = -i * precision;
        
        uint64_t resized = flr_resize_float_double(original, src_exp_bits, src_mant_bits, dst_exp_bits, dst_mant_bits);
        uint64_t restored = flr_resize_float_long(resized, dst_exp_bits, dst_mant_bits, src_exp_bits, src_mant_bits);
        qword wq;
        wq.uint64_val = restored;

        EXPECT_EQ((int) std::round(original * multiplier), (int) std::round(wq.double_val * multiplier));
    }
}

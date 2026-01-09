#include "irata2/microcode/encoder/status_encoder.h"

#include <algorithm>
#include <gtest/gtest.h>

using irata2::microcode::encoder::StatusEncoder;
using irata2::microcode::output::StatusBitDefinition;
using irata2::microcode::MicrocodeError;

TEST(StatusEncoderTest, ExpandsEmptyPartialWhenNoBitsConfigured) {
  StatusEncoder encoder({});
  auto values = encoder.ExpandPartial({});
  ASSERT_EQ(values.size(), 1u);
  EXPECT_EQ(values.front(), 0);
}

TEST(StatusEncoderTest, RejectsPartialWhenNoBitsConfigured) {
  StatusEncoder encoder({});
  EXPECT_THROW(encoder.ExpandPartial({{"zero", true}}), MicrocodeError);
}

TEST(StatusEncoderTest, ExpandsPartialStatuses) {
  StatusEncoder encoder({{"zero", 0}, {"negative", 7}});

  auto values = encoder.ExpandPartial({{"zero", true}});
  EXPECT_EQ(values.size(), 2u);
  EXPECT_NE(std::find(values.begin(), values.end(), 0x01), values.end());
  EXPECT_NE(std::find(values.begin(), values.end(), 0x81), values.end());
}

TEST(StatusEncoderTest, RejectsUnknownStatus) {
  StatusEncoder encoder({{"zero", 0}});
  EXPECT_THROW(encoder.ExpandPartial({{"carry", true}}), MicrocodeError);
}

TEST(StatusEncoderTest, RejectsOutOfRangeBitIndex) {
  EXPECT_THROW(StatusEncoder({{"zero", 8}}), MicrocodeError);
}

TEST(StatusEncoderTest, DecodesEmptyStatusWhenNoBitsConfigured) {
  StatusEncoder encoder({});
  const auto decoded = encoder.Decode(0);
  EXPECT_TRUE(decoded.empty());
}

TEST(StatusEncoderTest, RejectsNonZeroStatusWhenNoBitsConfigured) {
  StatusEncoder encoder({});
  EXPECT_THROW(encoder.Decode(0x01), MicrocodeError);
}

TEST(StatusEncoderTest, DecodesStatusBits) {
  StatusEncoder encoder({{"zero", 0}, {"negative", 7}});
  const auto decoded = encoder.Decode(0x80);
  const auto zero_it = decoded.find("zero");
  ASSERT_NE(zero_it, decoded.end());
  EXPECT_FALSE(zero_it->second);
  const auto negative_it = decoded.find("negative");
  ASSERT_NE(negative_it, decoded.end());
  EXPECT_TRUE(negative_it->second);
}

#include "irata2/microcode/encoder/instruction_encoder.h"

#include <gtest/gtest.h>

using irata2::microcode::encoder::InstructionEncoder;
using irata2::microcode::output::EncodeKey;

TEST(InstructionEncoderTest, DecodesEncodedKey) {
  auto key =
      InstructionEncoder::MakeKey(irata2::isa::Opcode::HLT_IMP, 2, 0xAA);
  const auto encoded = EncodeKey(key);
  const auto decoded = InstructionEncoder::DecodeKey(encoded);

  EXPECT_EQ(decoded.opcode, key.opcode);
  EXPECT_EQ(decoded.step, key.step);
  EXPECT_EQ(decoded.status, key.status);
}

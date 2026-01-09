#include "irata2/microcode/encoder/control_encoder.h"

#include "irata2/hdl/control.h"
#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <algorithm>
#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::encoder::ControlEncoder;

namespace {
class ExtraControl final
    : public irata2::hdl::Control<ExtraControl, void,
                                  irata2::base::TickPhase::Process, true> {
 public:
  ExtraControl(std::string name, irata2::hdl::ComponentBase& parent)
      : Control<ExtraControl, void, irata2::base::TickPhase::Process, true>(
            std::move(name), parent) {}
};
}  // namespace

TEST(ControlEncoderTest, EncodesControlBitsByPath) {
  Cpu cpu;
  ControlEncoder encoder(cpu);

  const auto& paths = encoder.control_paths();
  const auto it = std::find(paths.begin(), paths.end(), "halt");
  ASSERT_NE(it, paths.end());

  const size_t index = static_cast<size_t>(it - paths.begin());
  const uint64_t word = encoder.Encode({&cpu.halt()});
  EXPECT_EQ(word, uint64_t{1} << index);
}

TEST(ControlEncoderTest, DecodesControlWord) {
  Cpu cpu;
  ControlEncoder encoder(cpu);

  const uint64_t word = encoder.Encode({&cpu.halt(), &cpu.crash()});
  const auto decoded = encoder.Decode(word);

  EXPECT_NE(std::find(decoded.begin(), decoded.end(), "halt"), decoded.end());
  EXPECT_NE(std::find(decoded.begin(), decoded.end(), "crash"), decoded.end());
}

TEST(ControlEncoderTest, RejectsUnknownControl) {
  Cpu cpu;
  ControlEncoder encoder(cpu);
  ExtraControl extra("extra", cpu);

  EXPECT_THROW(encoder.Encode({&extra}), MicrocodeError);
}

#include "irata2/sim.h"
#include "irata2/sim/error.h"

#include "irata2/hdl.h"
#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/output/program.h"

#include <algorithm>
#include <memory>
#include <gtest/gtest.h>

using irata2::microcode::output::EncodeKey;
using irata2::microcode::output::MicrocodeKey;
using irata2::microcode::output::MicrocodeProgram;
using namespace irata2::sim;

namespace {
std::shared_ptr<MicrocodeProgram> MakeProgramWithControls(
    const irata2::hdl::Cpu& hdl,
    std::initializer_list<std::string_view> asserted_paths,
    uint8_t opcode = 0x01) {
  irata2::microcode::encoder::ControlEncoder encoder(hdl);
  auto program = std::make_shared<MicrocodeProgram>();
  program->control_paths = encoder.control_paths();

  __uint128_t word = 0;
  for (const auto& path : asserted_paths) {
    const auto it = std::find(program->control_paths.begin(),
                              program->control_paths.end(),
                              std::string(path));
    if (it == program->control_paths.end()) {
      throw SimError("control path not found in test HDL: " + std::string(path));
    }
    const size_t index = static_cast<size_t>(it - program->control_paths.begin());
    word |= (__uint128_t{1} << index);
  }

  MicrocodeKey key{opcode, 0, 0};
  program->table[EncodeKey(key)] = word;
  return program;
}
}  // namespace

TEST(SimControllerTest, AssertsHaltControlFromMicrocode) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = MakeProgramWithControls(*hdl, {"halt"});

  Cpu sim(hdl, program);
  sim.controller().ir().set_value(irata2::base::Byte{0x01});
  sim.controller().sc().set_value(irata2::base::Byte{0});

  sim.Tick();
  EXPECT_TRUE(sim.halted());
}

TEST(SimControllerTest, RejectsMissingMicrocodeEntry) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = std::make_shared<MicrocodeProgram>();
  irata2::microcode::encoder::ControlEncoder encoder(*hdl);
  program->control_paths = encoder.control_paths();

  Cpu sim(hdl, program);
  sim.controller().ir().set_value(irata2::base::Byte{0x01});
  sim.controller().sc().set_value(irata2::base::Byte{0});

  EXPECT_THROW(sim.Tick(), SimError);
}

TEST(SimControllerTest, RejectsUnknownControlPath) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = std::make_shared<MicrocodeProgram>();
  irata2::microcode::encoder::ControlEncoder encoder(*hdl);
  program->control_paths = encoder.control_paths();
  if (!program->control_paths.empty()) {
    program->control_paths[0] = "unknown";
  }

  EXPECT_THROW(Cpu sim(hdl, program), SimError);
}

TEST(SimControllerTest, RejectsControlWordOverflow) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = std::make_shared<MicrocodeProgram>();
  irata2::microcode::encoder::ControlEncoder encoder(*hdl);
  program->control_paths = encoder.control_paths();
  if (!program->control_paths.empty() &&
      program->control_paths.size() < 128) {
    MicrocodeKey key{0x01, 0, 0};
    program->table[EncodeKey(key)] =
        (__uint128_t{1} << program->control_paths.size());
  }

  Cpu sim(hdl, program);
  sim.controller().ir().set_value(irata2::base::Byte{0x01});
  sim.controller().sc().set_value(irata2::base::Byte{0});

  EXPECT_THROW(sim.Tick(), SimError);
}

TEST(SimControllerTest, InstructionStartCapturesPcValue) {
  // Test that IPC captures PC's value when instruction_start is asserted.
  // This matches fetch preamble step 0: [pc.write, controller.instruction_start, ...]
  // Note: pc.increment is in a separate step (step 2), not here.
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = MakeProgramWithControls(
      *hdl,
      {"pc.write", "controller.instruction_start"},
      0x02);

  Cpu sim(hdl, program);
  sim.pc().set_value(irata2::base::Word{0x8000});
  sim.controller().ir().set_value(irata2::base::Byte{0x02});
  sim.controller().sc().set_value(irata2::base::Byte{0});

  sim.Tick();

  // PC unchanged (no increment in this step)
  EXPECT_EQ(sim.pc().value().value(), 0x8000);
  // IPC captured PC's value
  EXPECT_EQ(sim.controller().ipc().value().value(), 0x8000);
}

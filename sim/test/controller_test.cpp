#include "irata2/sim/cpu.h"
#include "irata2/sim/error.h"

#include "irata2/hdl/cpu.h"
#include "irata2/microcode/output/program.h"

#include <memory>
#include <gtest/gtest.h>

using irata2::microcode::output::EncodeKey;
using irata2::microcode::output::MicrocodeKey;
using irata2::microcode::output::MicrocodeProgram;
using namespace irata2::sim;

namespace {
std::shared_ptr<MicrocodeProgram> MakeProgramWithControls(
    std::vector<std::string> control_paths) {
  auto program = std::make_shared<MicrocodeProgram>();
  program->control_paths = std::move(control_paths);
  return program;
}
}  // namespace

TEST(SimControllerTest, AssertsHaltControlFromMicrocode) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = MakeProgramWithControls({"/cpu/halt"});
  MicrocodeKey key{0x01, 0, 0};
  program->table[EncodeKey(key)] = 1;

  Cpu sim(hdl, program);
  sim.controller().ir().set_value(irata2::base::Byte{0x01});
  sim.controller().sc().set_value(irata2::base::Byte{0});

  sim.Tick();
  EXPECT_TRUE(sim.halted());
}

TEST(SimControllerTest, RejectsMissingMicrocodeEntry) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = MakeProgramWithControls({"/cpu/halt"});

  Cpu sim(hdl, program);
  sim.controller().ir().set_value(irata2::base::Byte{0x01});
  sim.controller().sc().set_value(irata2::base::Byte{0});

  EXPECT_THROW(sim.Tick(), SimError);
}

TEST(SimControllerTest, RejectsUnknownControlPath) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = MakeProgramWithControls({"/cpu/unknown"});

  EXPECT_THROW(Cpu sim(hdl, program), SimError);
}

TEST(SimControllerTest, RejectsControlWordOverflow) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = MakeProgramWithControls({"/cpu/halt"});
  MicrocodeKey key{0x01, 0, 0};
  program->table[EncodeKey(key)] = 2;

  Cpu sim(hdl, program);
  sim.controller().ir().set_value(irata2::base::Byte{0x01});
  sim.controller().sc().set_value(irata2::base::Byte{0});

  EXPECT_THROW(sim.Tick(), SimError);
}

#include "irata2/sim.h"

#include "irata2/hdl.h"
#include "irata2/microcode/output/program.h"

#include <memory>
#include <gtest/gtest.h>

using namespace irata2::sim;

namespace {
void SetSafeIr(Cpu& sim) {
  sim.controller().ir().set_value(irata2::base::Byte{0x02});
  sim.controller().sc().set_value(irata2::base::Byte{0});
}
}  // namespace

TEST(SimCounterTest, IncrementsWordCounter) {
  Cpu sim;

  SetSafeIr(sim);
  sim.pc().increment().Assert();
  sim.Tick();

  EXPECT_EQ(sim.pc().value(), irata2::base::Word{1});
}

TEST(SimCounterTest, ResetsCounter) {
  Cpu sim;

  SetSafeIr(sim);
  sim.pc().set_value(irata2::base::Word{0x1234});
  sim.pc().reset().Assert();
  sim.Tick();

  EXPECT_EQ(sim.pc().value(), irata2::base::Word{0});
}

TEST(SimCounterTest, IncrementsLocalCounter) {
  auto hdl = std::make_shared<irata2::hdl::Cpu>();
  auto program = std::make_shared<irata2::microcode::output::MicrocodeProgram>();
  irata2::microcode::output::MicrocodeKey key{0x02, 0, 0};
  program->table[irata2::microcode::output::EncodeKey(key)] = 0;

  Cpu sim(hdl, program);
  sim.controller().ir().set_value(irata2::base::Byte{0x02});
  sim.controller().sc().set_value(irata2::base::Byte{0});
  sim.controller().sc().increment().Assert();
  sim.Tick();

  EXPECT_EQ(sim.controller().sc().value(), irata2::base::Byte{1});
}

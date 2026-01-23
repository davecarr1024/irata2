#include "irata2/sim.h"
#include "irata2/sim/instruction_register.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(InstructionRegisterTest, InjectsIrqOpcodeAtInstructionStart) {
  Cpu sim;
  auto& ir = sim.controller().ir();

  test::SetPhase(sim, irata2::base::TickPhase::Control);
  sim.irq_line().Assert();
  sim.controller().instruction_start().Assert();
  ir.set_value(irata2::base::Byte{0xAB});

  test::SetPhase(sim, irata2::base::TickPhase::Process);
  ir.TickProcess();

  EXPECT_EQ(ir.value().value(), InstructionRegister::kIrqOpcode);
}

TEST(InstructionRegisterTest, UsesStoredOpcodeWhenNoIrq) {
  Cpu sim;
  auto& ir = sim.controller().ir();

  test::SetPhase(sim, irata2::base::TickPhase::Control);
  sim.irq_line().Set(false);
  sim.controller().instruction_start().Assert();
  ir.set_value(irata2::base::Byte{0xCD});

  test::SetPhase(sim, irata2::base::TickPhase::Process);
  ir.TickProcess();

  EXPECT_EQ(ir.value().value(), 0xCD);
}

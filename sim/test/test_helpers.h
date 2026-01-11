#ifndef IRATA2_SIM_TEST_HELPERS_H
#define IRATA2_SIM_TEST_HELPERS_H

#include <memory>

#include "irata2/sim.h"
#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/output/program.h"

namespace irata2::sim::test {

inline std::shared_ptr<const microcode::output::MicrocodeProgram> MakeNoopProgram() {
  auto program = std::make_shared<microcode::output::MicrocodeProgram>();
  irata2::microcode::encoder::ControlEncoder encoder(*DefaultHdl());
  program->control_paths = encoder.control_paths();
  auto add = [&](uint8_t opcode) {
    microcode::output::MicrocodeKey key{opcode, 0, 0};
    program->table.emplace(microcode::output::EncodeKey(key), 0);
  };
  add(0x00);
  add(0x01);
  add(0x02);
  add(0xFF);
  return program;
}

inline Cpu MakeTestCpu() {
  return Cpu(DefaultHdl(), MakeNoopProgram());
}

inline void SetPhase(Cpu& cpu, base::TickPhase phase) {
  cpu.SetCurrentPhaseForTest(phase);
}

template <typename ControlType>
void AssertControl(ControlType& control) {
  SetPhase(control.cpu(), base::TickPhase::Control);
  control.Assert();
}

template <typename ControlType>
void ClearControl(ControlType& control) {
  SetPhase(control.cpu(), base::TickPhase::Control);
  control.Clear();
}

template <typename ControlType>
bool IsAsserted(ControlType& control) {
  SetPhase(control.cpu(), control.phase());
  return control.asserted();
}

}  // namespace irata2::sim::test

#endif  // IRATA2_SIM_TEST_HELPERS_H

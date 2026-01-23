#ifndef IRATA2_SIM_CONTROLLER_H
#define IRATA2_SIM_CONTROLLER_H

#include <memory>
#include <utility>
#include <vector>

#include "irata2/microcode/output/program.h"
#include "irata2/sim/instruction_register.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/controller/instruction_memory.h"
#include "irata2/sim/local_counter.h"
#include "irata2/sim/latched_word_register.h"
#include "irata2/sim/program_counter.h"

namespace irata2::sim::controller {

class Controller final : public ComponentWithParent {
 public:
  Controller(std::string name,
             Component& parent,
             Bus<base::Byte>& data_bus,
             const ProgramCounter& pc);

  InstructionRegister& ir() { return ir_; }
  const InstructionRegister& ir() const { return ir_; }

  LocalCounter<base::Byte>& sc() { return sc_; }
  const LocalCounter<base::Byte>& sc() const { return sc_; }
  LatchedWordRegister& ipc() { return ipc_; }
  const LatchedWordRegister& ipc() const { return ipc_; }
  ProcessControl<true>& instruction_start() { return instruction_start_; }
  const ProcessControl<true>& instruction_start() const {
    return instruction_start_;
  }

  void LoadProgram(
      std::shared_ptr<const microcode::output::MicrocodeProgram> program);

  const InstructionMemory* instruction_memory() const {
    return instruction_memory_.get();
  }

  void TickControl() override;
  void TickProcess() override;

 private:
  ProcessControl<true> instruction_start_;
  InstructionRegister ir_;
  LocalCounter<base::Byte> sc_;
  const ProgramCounter& pc_;
  LatchedWordRegister ipc_;
  std::unique_ptr<InstructionMemory> instruction_memory_;
};

}  // namespace irata2::sim::controller

#endif  // IRATA2_SIM_CONTROLLER_H

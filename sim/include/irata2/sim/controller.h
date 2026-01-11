#ifndef IRATA2_SIM_CONTROLLER_H
#define IRATA2_SIM_CONTROLLER_H

#include <memory>
#include <utility>
#include <vector>

#include "irata2/microcode/output/program.h"
#include "irata2/sim/byte_register.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/local_counter.h"

namespace irata2::sim {

class Controller final : public ComponentWithParent {
 public:
  Controller(std::string name, Component& parent, Bus<base::Byte>& data_bus);

  ByteRegister& ir() { return ir_; }
  const ByteRegister& ir() const { return ir_; }

  LocalCounter<base::Byte>& sc() { return sc_; }
  const LocalCounter<base::Byte>& sc() const { return sc_; }

  void LoadProgram(
      std::shared_ptr<const microcode::output::MicrocodeProgram> program);
  const microcode::output::MicrocodeProgram* program() const {
    return program_.get();
  }

  void TickControl() override;

 private:
  uint64_t LookupControlWord(uint8_t opcode, uint8_t step, uint8_t status) const;
  void AssertControlWord(uint64_t control_word);
  uint8_t EncodeStatus() const;

  ByteRegister ir_;
  LocalCounter<base::Byte> sc_;
  std::shared_ptr<const microcode::output::MicrocodeProgram> program_;
  std::vector<ControlBase*> control_lines_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CONTROLLER_H

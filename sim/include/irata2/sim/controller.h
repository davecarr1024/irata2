#ifndef IRATA2_SIM_CONTROLLER_H
#define IRATA2_SIM_CONTROLLER_H

#include <utility>

#include "irata2/sim/byte_register.h"
#include "irata2/sim/component.h"
#include "irata2/sim/local_counter.h"

namespace irata2::sim {

class Controller final : public ComponentWithParent {
 public:
  Controller(std::string name, Component& parent, Bus<base::Byte>& data_bus);

  ByteRegister& ir() { return ir_; }
  const ByteRegister& ir() const { return ir_; }

  LocalCounter<base::Byte>& sc() { return sc_; }
  const LocalCounter<base::Byte>& sc() const { return sc_; }

 private:
  ByteRegister ir_;
  LocalCounter<base::Byte> sc_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CONTROLLER_H

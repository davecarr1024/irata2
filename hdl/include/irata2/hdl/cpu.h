#ifndef IRATA2_HDL_CPU_H
#define IRATA2_HDL_CPU_H

#include "irata2/hdl/component.h"

namespace irata2::hdl {

// Root HDL component representing the CPU structure
// This is immutable structural metadata - no runtime state
class Cpu : public Component {
 public:
  Cpu();

  const Cpu& cpu() const override { return *this; }

  std::string path() const override { return "/cpu"; }

  // Component accessors will be added here as we build the system
  // For example:
  // const ByteBus& data_bus() const { return data_bus_; }
  // const ByteRegister& a() const { return a_; }
  // const Memory& memory() const { return memory_; }

 private:
  // Component members will be added here
  // const ByteBus data_bus_;
  // const WordBus address_bus_;
  // const ByteRegister a_;
  // const Memory memory_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CPU_H

#include "irata2/hdl/cpu.h"

namespace irata2::hdl {

Cpu::Cpu()
    : Component<Cpu>("cpu", this),
      halt_control_("halt", *this),
      crash_control_("crash", *this),
      data_bus_("data_bus", *this),
      address_bus_("address_bus", *this),
      a_("a", *this, data_bus_),
      x_("x", *this, data_bus_),
      y_("y", *this, data_bus_),
      tmp_("tmp", *this, address_bus_),
      alu_("alu", *this, data_bus_),
      pc_("pc", *this, address_bus_, data_bus_),
      status_("status", *this, data_bus_),
      controller_("controller", *this, data_bus_),
      memory_("memory", *this, data_bus_, address_bus_) {}

const Cpu& GetCpu() {
  // C++11 guarantees thread-safe initialization of static locals
  static const Cpu instance;
  return instance;
}

}  // namespace irata2::hdl

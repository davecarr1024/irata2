#include "irata2/hdl/cpu.h"

namespace irata2::hdl {

Cpu::Cpu()
    : Component<Cpu>("cpu", this),
      data_bus_("data_bus", *this),
      address_bus_("address_bus", *this),
      a_("a", *this, data_bus_),
      x_("x", *this, data_bus_),
      pc_("pc", *this, address_bus_),
      mar_("mar", *this, address_bus_) {}

}  // namespace irata2::hdl

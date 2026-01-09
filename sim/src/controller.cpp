#include "irata2/sim/controller.h"

namespace irata2::sim {

Controller::Controller(std::string name,
                       Component& parent,
                       Bus<base::Byte>& data_bus)
    : ComponentWithParent(parent, std::move(name)),
      ir_("ir", *this, data_bus),
      sc_("sc", *this) {}

}  // namespace irata2::sim

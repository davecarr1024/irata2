#include "irata2/hdl/controller.h"

namespace irata2::hdl {

Controller::Controller(std::string name,
                       ComponentBase& parent,
                       const ByteBus& data_bus)
    : ComponentWithParent<Controller>(std::move(name), parent),
      ir_("ir", *this, data_bus),
      sc_("sc", *this) {}

}  // namespace irata2::hdl

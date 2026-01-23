#ifndef IRATA2_HDL_LATCHED_CONTROL_H
#define IRATA2_HDL_LATCHED_CONTROL_H

#include "irata2/base/tick_phase.h"
#include "irata2/hdl/control.h"

#include <utility>

namespace irata2::hdl {

template <base::TickPhase Phase>
class LatchedControl final
    : public Control<LatchedControl<Phase>, void, Phase, false> {
 public:
  LatchedControl(std::string name, ComponentBase& parent)
      : Control<LatchedControl<Phase>, void, Phase, false>(
            std::move(name), parent) {}
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_LATCHED_CONTROL_H

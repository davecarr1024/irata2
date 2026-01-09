#ifndef IRATA2_HDL_PROCESS_CONTROL_H
#define IRATA2_HDL_PROCESS_CONTROL_H

#include "irata2/hdl/control.h"

#include <utility>

namespace irata2::hdl {

template <bool AutoReset = true>
class ProcessControl final
    : public Control<ProcessControl<AutoReset>, void, base::TickPhase::Process,
                     AutoReset> {
 public:
  ProcessControl(std::string name, ComponentBase& parent)
      : Control<ProcessControl<AutoReset>, void, base::TickPhase::Process,
                AutoReset>(std::move(name), parent) {}
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_PROCESS_CONTROL_H

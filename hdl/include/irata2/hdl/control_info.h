#ifndef IRATA2_HDL_CONTROL_INFO_H
#define IRATA2_HDL_CONTROL_INFO_H

#include "irata2/base/tick_phase.h"

#include <string_view>

namespace irata2::hdl {

/// Non-virtual struct that holds pre-computed control properties.
/// This replaces the virtual ControlBase interface with a POD-like type.
/// Each Control stores a ControlInfo member that is populated at construction.
struct ControlInfo {
  base::TickPhase phase;
  bool auto_reset;
  std::string_view path;  // Points to stable ComponentBase::path_

  constexpr bool operator==(const ControlInfo& other) const {
    return phase == other.phase && auto_reset == other.auto_reset &&
           path == other.path;
  }

  constexpr bool operator!=(const ControlInfo& other) const {
    return !(*this == other);
  }
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CONTROL_INFO_H

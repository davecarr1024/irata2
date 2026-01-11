#ifndef IRATA2_HDL_CONTROL_INFO_H
#define IRATA2_HDL_CONTROL_INFO_H

#include "irata2/base/tick_phase.h"

#include <string_view>

namespace irata2::hdl {

/**
 * @brief Non-virtual metadata for HDL control signals.
 *
 * ControlInfo stores pre-computed control properties as a POD-like struct,
 * eliminating virtual dispatch overhead. Each Control template instantiation
 * creates a ControlInfo at construction time.
 *
 * This design enables zero-cost abstractions: control properties are known
 * at compile-time via template parameters, and ControlInfo provides runtime
 * access without vtable lookups.
 *
 * @see Control for the CRTP control template
 * @see microcode::ir::CpuPathResolver for control lookup by path
 */
struct ControlInfo {
  base::TickPhase phase;   ///< Phase during which this control is active
  bool auto_reset;         ///< Whether control auto-clears after each tick
  std::string_view path;   ///< Component path (points to stable storage)

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

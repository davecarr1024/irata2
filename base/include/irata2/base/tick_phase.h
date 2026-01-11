#ifndef IRATA2_BASE_TICK_PHASE_H
#define IRATA2_BASE_TICK_PHASE_H

#include <string>

namespace irata2::base {

/**
 * @brief Five-phase tick model for CPU execution.
 *
 * Each CPU clock cycle is divided into five sequential phases that
 * enforce correct ordering of operations. This models how real
 * hardware uses clock edges and propagation delays.
 *
 * Phase order:
 * 1. **Control** - Controller reads microcode ROM and asserts control signals
 * 2. **Write** - Components with asserted write controls output to buses
 * 3. **Read** - Components with asserted read controls sample from buses
 * 4. **Process** - Internal updates (ALU computation, status flag updates)
 * 5. **Clear** - Auto-reset controls return to de-asserted state
 *
 * Controls validate their phase during access, preventing subtle timing bugs.
 *
 * @see sim::Cpu::Tick() for phase execution
 * @see sim::ControlBase for phase validation
 */
enum class TickPhase {
  None,     ///< Not currently in a tick
  Control,  ///< Controller reads microcode and asserts control signals
  Write,    ///< Components write to buses
  Read,     ///< Components read from buses
  Process,  ///< Internal component updates (ALU, status flags)
  Clear     ///< Reset auto-clear controls
};

/**
 * @brief Convert TickPhase to human-readable string.
 * @param phase The phase to convert
 * @return String representation for debugging/logging
 */
inline std::string ToString(TickPhase phase) {
  switch (phase) {
    case TickPhase::None:
      return "None";
    case TickPhase::Control:
      return "Control";
    case TickPhase::Write:
      return "Write";
    case TickPhase::Read:
      return "Read";
    case TickPhase::Process:
      return "Process";
    case TickPhase::Clear:
      return "Clear";
  }
  return "Unknown";
}

}  // namespace irata2::base

#endif  // IRATA2_BASE_TICK_PHASE_H

#ifndef IRATA2_BASE_TICK_PHASE_H
#define IRATA2_BASE_TICK_PHASE_H

#include <string>

namespace irata2::base {

// Five-phase tick model for CPU execution
enum class TickPhase {
  None,     // Not currently in a tick
  Control,  // Controller reads microcode and asserts control signals
  Write,    // Components write to buses
  Read,     // Components read from buses
  Process,  // Internal component updates (ALU, status flags)
  Clear     // Reset auto-clear controls
};

// Convert TickPhase to string for debugging
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

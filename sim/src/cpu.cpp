#include "irata2/sim/cpu.h"

namespace irata2::sim {

Cpu::Cpu(const hdl::Cpu& hdl) : hdl_(hdl) {
  // Constructor for simulator CPU
  // Components will be initialized here as we build the system
}

void Cpu::Tick() {
  if (halted_) {
    return;
  }

  // Execute five-phase tick model
  current_phase_ = base::TickPhase::Control;
  TickControl();

  current_phase_ = base::TickPhase::Write;
  TickWrite();

  current_phase_ = base::TickPhase::Read;
  TickRead();

  current_phase_ = base::TickPhase::Process;
  TickProcess();

  current_phase_ = base::TickPhase::Clear;
  TickClear();

  current_phase_ = base::TickPhase::None;
  cycle_count_++;
}

}  // namespace irata2::sim

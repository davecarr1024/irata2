#include "irata2/sim/cpu.h"

namespace irata2::sim {

Cpu::Cpu(const hdl::Cpu& hdl)
    : hdl_(hdl),
      halt_control_("halt", *this),
      crash_control_("crash", *this),
      data_bus_("data_bus", *this),
      address_bus_("address_bus", *this),
      a_("a", *this, data_bus_),
      x_("x", *this, data_bus_),
      pc_("pc", *this, address_bus_),
      mar_("mar", *this, address_bus_),
      status_("status", *this, data_bus_),
      controller_("controller", *this, data_bus_) {
  RegisterChild(halt_control_);
  RegisterChild(crash_control_);
  RegisterChild(data_bus_);
  RegisterChild(address_bus_);

  RegisterChild(a_);
  RegisterChild(a_.write());
  RegisterChild(a_.read());
  RegisterChild(a_.reset());

  RegisterChild(x_);
  RegisterChild(x_.write());
  RegisterChild(x_.read());
  RegisterChild(x_.reset());

  RegisterChild(pc_);
  RegisterChild(pc_.write());
  RegisterChild(pc_.read());
  RegisterChild(pc_.reset());
  RegisterChild(pc_.increment());

  RegisterChild(mar_);
  RegisterChild(mar_.write());
  RegisterChild(mar_.read());
  RegisterChild(mar_.reset());

  RegisterChild(status_);
  RegisterChild(status_.write());
  RegisterChild(status_.read());
  RegisterChild(status_.reset());

  RegisterChild(controller_);
  RegisterChild(controller_.ir());
  RegisterChild(controller_.ir().write());
  RegisterChild(controller_.ir().read());
  RegisterChild(controller_.ir().reset());
  RegisterChild(controller_.sc());
  RegisterChild(controller_.sc().reset());
  RegisterChild(controller_.sc().increment());
}

void Cpu::RegisterChild(Component& child) {
  components_.push_back(&child);
}

void Cpu::TickPhase(void (Component::*phase)()) {
  for (auto* component : components_) {
    (component->*phase)();
  }
  (this->*phase)();
}

void Cpu::Tick() {
  if (halted_) {
    return;
  }

  // Execute five-phase tick model
  current_phase_ = base::TickPhase::Control;
  TickPhase(&Component::TickControl);

  current_phase_ = base::TickPhase::Write;
  TickPhase(&Component::TickWrite);

  current_phase_ = base::TickPhase::Read;
  TickPhase(&Component::TickRead);

  current_phase_ = base::TickPhase::Process;
  TickPhase(&Component::TickProcess);

  current_phase_ = base::TickPhase::Clear;
  TickPhase(&Component::TickClear);

  current_phase_ = base::TickPhase::None;
  cycle_count_++;
}

void Cpu::TickProcess() {
  if (halt_control_.asserted()) {
    halted_ = true;
  }
  if (crash_control_.asserted()) {
    crashed_ = true;
    halted_ = true;
  }
}

}  // namespace irata2::sim

#include "irata2/sim/cpu.h"

#include "irata2/sim/error.h"
#include "irata2/sim/initialization.h"

#include <algorithm>

namespace irata2::sim {

Cpu::Cpu() : Cpu(DefaultHdl(), DefaultMicrocodeProgram()) {}

Cpu::Cpu(std::shared_ptr<const hdl::Cpu> hdl,
         std::shared_ptr<const microcode::output::MicrocodeProgram> program)
    : hdl_(std::move(hdl)),
      microcode_(std::move(program)),
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
  if (!hdl_) {
    throw SimError("cpu constructed without HDL");
  }
  if (!microcode_) {
    throw SimError("cpu constructed without microcode program");
  }

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

  controller_.LoadProgram(microcode_);
}

void Cpu::RegisterChild(Component& child) {
  components_.push_back(&child);
  controls_indexed_ = false;
}

namespace {
std::string NormalizePath(std::string_view path) {
  if (path.empty()) {
    return {};
  }

  if (!path.empty() && path.front() == '/') {
    return std::string(path);
  }

  std::string normalized;
  normalized.reserve(path.size() + 5);
  normalized = "/cpu/";
  for (char ch : path) {
    normalized.push_back(ch == '.' ? '/' : ch);
  }
  return normalized;
}
}  // namespace

void Cpu::IndexControls() const {
  if (controls_indexed_) {
    return;
  }

  controls_by_path_.clear();
  control_paths_.clear();

  for (auto* component : components_) {
    if (auto* control = dynamic_cast<ControlBase*>(component)) {
      auto [it, inserted] =
          controls_by_path_.emplace(component->path(), control);
      if (!inserted) {
        throw SimError("duplicate control path in sim: " + component->path());
      }
      control_paths_.push_back(component->path());
    }
  }

  std::sort(control_paths_.begin(), control_paths_.end());
  controls_indexed_ = true;
}

ControlBase* Cpu::ResolveControl(std::string_view path) {
  return const_cast<ControlBase*>(
      static_cast<const Cpu&>(*this).ResolveControl(path));
}

const ControlBase* Cpu::ResolveControl(std::string_view path) const {
  const std::string normalized = NormalizePath(path);
  if (normalized.empty()) {
    throw SimError("control path is empty");
  }

  IndexControls();
  const auto it = controls_by_path_.find(normalized);
  if (it == controls_by_path_.end()) {
    throw SimError("control path not found in sim: " + normalized);
  }
  return it->second;
}

std::vector<std::string> Cpu::AllControlPaths() const {
  IndexControls();
  return control_paths_;
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

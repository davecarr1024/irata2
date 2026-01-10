#include "irata2/sim/cpu.h"

#include "irata2/sim/error.h"
#include "irata2/sim/initialization.h"

#include <algorithm>

namespace irata2::sim {

Cpu::Cpu() : Cpu(DefaultHdl(), DefaultMicrocodeProgram()) {}

Cpu::Cpu(std::shared_ptr<const hdl::Cpu> hdl,
         std::shared_ptr<const microcode::output::MicrocodeProgram> program,
         std::shared_ptr<memory::Module> cartridge_rom,
         std::vector<memory::Region> extra_regions)
    : hdl_(std::move(hdl)),
      microcode_(std::move(program)),
      halt_control_("halt", *this),
      crash_control_("crash", *this),
      data_bus_("data_bus", *this),
      address_bus_("address_bus", *this),
      a_("a", *this, data_bus_),
      x_("x", *this, data_bus_),
      pc_("pc", *this, address_bus_),
      status_("status", *this, data_bus_),
      controller_("controller", *this, data_bus_),
      memory_("memory",
              *this,
              data_bus_,
              address_bus_,
              [&]() {
                std::vector<memory::Region> regions;
                regions.reserve(extra_regions.size() + 2);
                regions.emplace_back("ram",
                                     base::Word{0x0000},
                                     memory::MakeRam(0x2000));
                if (!cartridge_rom) {
                  cartridge_rom = memory::MakeRom(0x8000);
                }
                regions.emplace_back("cartridge",
                                     base::Word{0x8000},
                                     cartridge_rom);
                for (auto& region : extra_regions) {
                  regions.push_back(std::move(region));
                }
                return regions;
              }()) {
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

  RegisterChild(memory_);
  RegisterChild(memory_.write());
  RegisterChild(memory_.read());
  RegisterChild(memory_.mar());
  RegisterChild(memory_.mar().write());
  RegisterChild(memory_.mar().read());
  RegisterChild(memory_.mar().reset());
  RegisterChild(memory_.mar().low());
  RegisterChild(memory_.mar().low().write());
  RegisterChild(memory_.mar().low().read());
  RegisterChild(memory_.mar().low().reset());
  RegisterChild(memory_.mar().high());
  RegisterChild(memory_.mar().high().write());
  RegisterChild(memory_.mar().high().read());
  RegisterChild(memory_.mar().high().reset());

  BuildControlIndex();
  controller_.LoadProgram(microcode_);
  controller_.ir().set_value(base::Byte{0x02});
  controller_.sc().set_value(base::Byte{0});
}

void Cpu::RegisterChild(Component& child) {
  components_.push_back(&child);
}

void Cpu::BuildControlIndex() {
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
}

ControlBase* Cpu::ResolveControl(std::string_view path) {
  return const_cast<ControlBase*>(
      static_cast<const Cpu&>(*this).ResolveControl(path));
}

const ControlBase* Cpu::ResolveControl(std::string_view path) const {
  if (path.empty()) {
    throw SimError("control path is empty");
  }

  const auto it = controls_by_path_.find(std::string(path));
  if (it == controls_by_path_.end()) {
    throw SimError("control path not found in sim: " + std::string(path));
  }
  return it->second;
}

std::vector<std::string> Cpu::AllControlPaths() const {
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

Cpu::RunResult Cpu::RunUntilHalt() {
  while (!halted_) {
    Tick();
  }
  return {.halted = halted_, .crashed = crashed_};
}

void Cpu::SetCurrentPhaseForTest(base::TickPhase phase) {
  current_phase_ = phase;
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

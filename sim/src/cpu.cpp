#include "irata2/sim/cpu.h"

#include "irata2/sim/error.h"
#include "irata2/sim/initialization.h"
#include "irata2/microcode/compiler/compiler.h"
#include "irata2/microcode/ir/irata_instruction_set.h"
#include "irata2/hdl/traits.h"

#include <algorithm>
#include <sstream>

namespace irata2::sim {

namespace {
using irata2::microcode::output::MicrocodeProgram;
using irata2::microcode::output::StatusBitDefinition;

std::vector<StatusBitDefinition> BuildStatusBits(const hdl::StatusRegister& status) {
  return {
      {status.negative().name(), static_cast<uint8_t>(status.negative().bit_index())},
      {status.overflow().name(), static_cast<uint8_t>(status.overflow().bit_index())},
      {status.unused().name(), static_cast<uint8_t>(status.unused().bit_index())},
      {status.brk().name(), static_cast<uint8_t>(status.brk().bit_index())},
      {status.decimal().name(), static_cast<uint8_t>(status.decimal().bit_index())},
      {status.interrupt_disable().name(),
       static_cast<uint8_t>(status.interrupt_disable().bit_index())},
      {status.zero().name(), static_cast<uint8_t>(status.zero().bit_index())},
      {status.carry().name(), static_cast<uint8_t>(status.carry().bit_index())},
  };
}

MicrocodeProgram BuildMicrocodeProgram(const hdl::Cpu& hdl) {
  microcode::ir::InstructionSet instruction_set =
      microcode::ir::BuildIrataInstructionSet(hdl);

  microcode::encoder::ControlEncoder control_encoder(hdl);
  microcode::encoder::StatusEncoder status_encoder(BuildStatusBits(hdl.status()));
  microcode::compiler::Compiler compiler(control_encoder,
                                         status_encoder,
                                         hdl.controller().sc().increment().control_info(),
                                         hdl.controller().sc().reset().control_info());
  return compiler.Compile(std::move(instruction_set));
}

std::vector<memory::Memory::RegionFactory> BuildRegionFactories(
    std::vector<base::Byte> cartridge_rom,
    std::vector<memory::Memory::RegionFactory> extra_region_factories) {
  std::vector<memory::Memory::RegionFactory> factories;
  factories.reserve(2 + extra_region_factories.size());

  // RAM region at 0x0000
  factories.push_back([](memory::Memory& mem) -> std::unique_ptr<memory::Region> {
    return std::make_unique<memory::Region>(
        "ram", mem, base::Word{0x0000},
        [](memory::Region& reg) -> std::unique_ptr<memory::Module> {
          return std::make_unique<memory::Ram>("ram", reg, 0x2000,
                                                base::Byte{0x00});
        });
  });

  // Cartridge ROM region at 0x8000
  factories.push_back([rom_data = std::move(cartridge_rom)](
                          memory::Memory& mem) mutable -> std::unique_ptr<memory::Region> {
    return std::make_unique<memory::Region>(
        "cartridge", mem, base::Word{0x8000},
        [rom_data = std::move(rom_data)](
            memory::Region& reg) mutable -> std::unique_ptr<memory::Module> {
          if (rom_data.empty()) {
            // Default empty ROM
            return std::make_unique<memory::Rom>("rom", reg, 0x8000,
                                                  base::Byte{0xFF});
          }
          return std::make_unique<memory::Rom>("rom", reg, std::move(rom_data));
        });
  });

  // Add extra region factories
  for (auto& factory : extra_region_factories) {
    factories.push_back(std::move(factory));
  }

  return factories;
}
}  // namespace

std::shared_ptr<const hdl::Cpu> Cpu::GetDefaultHdl() {
  static const auto hdl = std::make_shared<const hdl::Cpu>();
  return hdl;
}

std::shared_ptr<const microcode::output::MicrocodeProgram> Cpu::GetDefaultMicrocodeProgram() {
  static const auto program = []() {
    auto built = BuildMicrocodeProgram(*GetDefaultHdl());
    return std::make_shared<const microcode::output::MicrocodeProgram>(std::move(built));
  }();
  return program;
}

Cpu::Cpu() : Cpu(GetDefaultHdl(), GetDefaultMicrocodeProgram()) {}

Cpu::Cpu(std::shared_ptr<const hdl::Cpu> hdl,
         std::shared_ptr<const microcode::output::MicrocodeProgram> program,
         std::vector<base::Byte> cartridge_rom,
         std::vector<memory::Memory::RegionFactory> extra_region_factories)
    : hdl_(std::move(hdl)),
      microcode_(std::move(program)),
      halt_control_("halt", *this),
      crash_control_("crash", *this),
      data_bus_("data_bus", *this),
      address_bus_("address_bus", *this),
      a_("a", *this, data_bus_),
      x_("x", *this, data_bus_),
      y_("y", *this, data_bus_),
      sp_("sp", *this, data_bus_),
      tmp_("tmp", *this, address_bus_),
      pc_("pc", *this, address_bus_, data_bus_),
      status_("status", *this, data_bus_),
      alu_("alu", *this, data_bus_, status_),
      controller_("controller", *this, data_bus_, address_bus_),
      memory_("memory",
              *this,
              data_bus_,
              address_bus_,
              BuildRegionFactories(std::move(cartridge_rom),
                                   std::move(extra_region_factories))) {
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

  RegisterChild(y_);
  RegisterChild(y_.write());
  RegisterChild(y_.read());
  RegisterChild(y_.reset());

  RegisterChild(sp_);
  RegisterChild(sp_.write());
  RegisterChild(sp_.read());
  RegisterChild(sp_.reset());
  RegisterChild(sp_.increment());
  RegisterChild(sp_.decrement());

  RegisterChild(tmp_);
  RegisterChild(tmp_.write());
  RegisterChild(tmp_.read());
  RegisterChild(tmp_.reset());

  RegisterChild(alu_);
  RegisterChild(alu_.lhs());
  RegisterChild(alu_.lhs().write());
  RegisterChild(alu_.lhs().read());
  RegisterChild(alu_.lhs().reset());
  RegisterChild(alu_.rhs());
  RegisterChild(alu_.rhs().write());
  RegisterChild(alu_.rhs().read());
  RegisterChild(alu_.rhs().reset());
  RegisterChild(alu_.result());
  RegisterChild(alu_.result().write());
  RegisterChild(alu_.result().read());
  RegisterChild(alu_.result().reset());
  RegisterChild(alu_.opcode_bit_0());
  RegisterChild(alu_.opcode_bit_1());
  RegisterChild(alu_.opcode_bit_2());
  RegisterChild(alu_.opcode_bit_3());

  RegisterChild(pc_);
  RegisterChild(pc_.write());
  RegisterChild(pc_.read());
  RegisterChild(pc_.reset());
  RegisterChild(pc_.low());
  RegisterChild(pc_.low().write());
  RegisterChild(pc_.low().read());
  RegisterChild(pc_.low().reset());
  RegisterChild(pc_.high());
  RegisterChild(pc_.high().write());
  RegisterChild(pc_.high().read());
  RegisterChild(pc_.high().reset());
  RegisterChild(pc_.signed_offset());
  RegisterChild(pc_.signed_offset().write());
  RegisterChild(pc_.signed_offset().read());
  RegisterChild(pc_.signed_offset().reset());
  RegisterChild(pc_.increment());
  RegisterChild(pc_.add_signed_offset());

  RegisterChild(status_);
  RegisterChild(status_.write());
  RegisterChild(status_.read());
  RegisterChild(status_.reset());
  RegisterChild(status_.negative());
  RegisterChild(status_.negative().set());
  RegisterChild(status_.negative().clear());
  RegisterChild(status_.overflow());
  RegisterChild(status_.overflow().set());
  RegisterChild(status_.overflow().clear());
  RegisterChild(status_.unused());
  RegisterChild(status_.unused().set());
  RegisterChild(status_.unused().clear());
  RegisterChild(status_.brk());
  RegisterChild(status_.brk().set());
  RegisterChild(status_.brk().clear());
  RegisterChild(status_.decimal());
  RegisterChild(status_.decimal().set());
  RegisterChild(status_.decimal().clear());
  RegisterChild(status_.interrupt_disable());
  RegisterChild(status_.interrupt_disable().set());
  RegisterChild(status_.interrupt_disable().clear());
  RegisterChild(status_.zero());
  RegisterChild(status_.zero().set());
  RegisterChild(status_.zero().clear());
  RegisterChild(status_.carry());
  RegisterChild(status_.carry().set());
  RegisterChild(status_.carry().clear());
  RegisterChild(status_.analyzer());
  RegisterChild(status_.analyzer().write());
  RegisterChild(status_.analyzer().read());
  RegisterChild(status_.analyzer().reset());

  RegisterChild(controller_);
  RegisterChild(controller_.ir());
  RegisterChild(controller_.ir().write());
  RegisterChild(controller_.ir().read());
  RegisterChild(controller_.ir().reset());
  RegisterChild(controller_.sc());
  RegisterChild(controller_.sc().reset());
  RegisterChild(controller_.sc().increment());
  RegisterChild(controller_.sc().decrement());
  RegisterChild(controller_.ipc());
  RegisterChild(controller_.ipc().latch());

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
  RegisterChild(memory_.mar().offset());
  RegisterChild(memory_.mar().offset().write());
  RegisterChild(memory_.mar().offset().read());
  RegisterChild(memory_.mar().offset().reset());
  RegisterChild(memory_.mar().add_offset());
  RegisterChild(memory_.mar().increment());
  RegisterChild(memory_.mar().stack_page());

  BuildControlIndex();
  ValidateAgainstHdl();
  controller_.LoadProgram(microcode_);
  controller_.ir().set_value(base::Byte{0x02});
  controller_.sc().set_value(base::Byte{0});
}

void Cpu::RegisterChild(Component& child) {
  // Call base class to add to children_ for tick propagation
  Component::RegisterChild(child);
  // Also add to components_ for control indexing
  components_.push_back(&child);
}

void Cpu::BuildControlIndex() {
  controls_by_path_.clear();
  control_paths_.clear();
  control_order_.clear();

  for (auto* component : components_) {
    if (auto* control = dynamic_cast<ControlBase*>(component)) {
      auto [it, inserted] =
          controls_by_path_.emplace(component->path(), control);
      if (!inserted) {
        throw SimError("duplicate control path in sim: " + component->path());
      }
      control_paths_.push_back(component->path());
      control_order_.push_back(control);
    }
  }
}

void Cpu::ValidateAgainstHdl() {
  // Collect all HDL control paths via visitor
  std::vector<std::string> hdl_paths;
  hdl_->visit([&](const auto& component) {
    using T = std::decay_t<decltype(component)>;
    if constexpr (hdl::is_control_v<T>) {
      hdl_paths.push_back(component.path());
    }
  });

  // Check that sim has at least as many controls as HDL
  if (control_paths_.size() < hdl_paths.size()) {
    std::ostringstream message;
    message << "sim has fewer controls (" << control_paths_.size()
            << ") than HDL (" << hdl_paths.size() << ")";
    throw SimError(message.str());
  }

  // Check that all HDL controls exist in sim with correct order
  for (size_t i = 0; i < hdl_paths.size(); ++i) {
    const auto& hdl_path = hdl_paths[i];

    // Check existence
    if (controls_by_path_.find(hdl_path) == controls_by_path_.end()) {
      throw SimError("HDL control not found in sim: " + hdl_path);
    }

    // Check order (first N controls in sim should match HDL order)
    if (i < control_paths_.size() && control_paths_[i] != hdl_path) {
      std::ostringstream message;
      message << "control order mismatch at index " << i
              << ": HDL has '" << hdl_path
              << "' but sim has '" << control_paths_[i] << "'";
      throw SimError(message.str());
    }
  }
}

ControlBase* Cpu::ResolveControl(std::string_view path) {
  if (path.empty()) {
    throw SimError("control path is empty");
  }
  const auto it = controls_by_path_.find(std::string(path));
  if (it == controls_by_path_.end()) {
    throw SimError("control path not found in sim: " + std::string(path));
  }
  return it->second;
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

void Cpu::Tick() {
  if (halted_) {
    return;
  }

  // Execute five-phase tick model
  // Each phase automatically propagates to all children via Component base class
  current_phase_ = base::TickPhase::Control;
  Component::TickControl();

  current_phase_ = base::TickPhase::Write;
  Component::TickWrite();

  current_phase_ = base::TickPhase::Read;
  Component::TickRead();

  current_phase_ = base::TickPhase::Process;
  TickProcess();  // Call our override which checks halt/crash controls

  current_phase_ = base::TickPhase::Clear;
  Component::TickClear();

  current_phase_ = base::TickPhase::None;
  cycle_count_++;
}

Cpu::CpuState Cpu::CaptureState() const {
  CpuState state;
  state.a = a_.value();
  state.x = x_.value();
  state.y = y_.value();
  state.sp = sp_.value();
  state.tmp = tmp_.value();
  state.pc = pc_.value();
  state.ir = controller_.ir().value();
  state.sc = controller_.sc().value();
  state.status = status_.value();
  state.cycle_count = cycle_count_;
  return state;
}

Cpu::RunResult Cpu::RunUntilHalt() {
  while (!halted_) {
    Tick();
  }

  RunResult result;
  result.cycles = cycle_count_;
  result.reason = crashed_ ? HaltReason::Crash : HaltReason::Halt;
  return result;
}

Cpu::RunResult Cpu::RunUntilHalt(uint64_t max_cycles, bool capture_state) {
  const uint64_t start_cycles = cycle_count_;

  while (!halted_ && (cycle_count_ - start_cycles) < max_cycles) {
    Tick();
  }

  RunResult result;
  result.cycles = cycle_count_ - start_cycles;

  if (halted_) {
    result.reason = crashed_ ? HaltReason::Crash : HaltReason::Halt;
  } else {
    result.reason = HaltReason::Timeout;
  }

  if (capture_state) {
    result.state = CaptureState();
  }

  return result;
}

void Cpu::EnableTrace(size_t depth) {
  trace_.Configure(depth);
}

base::Word Cpu::instruction_address() const {
  if (ipc_valid_) {
    return controller_.ipc().value();
  }
  return pc_.value();
}

std::optional<SourceLocation> Cpu::instruction_source_location() const {
  if (!debug_symbols_) {
    return std::nullopt;
  }
  return debug_symbols_->Lookup(instruction_address());
}

void Cpu::SetIpcForTest(base::Word address) {
  ipc_valid_ = true;
  controller_.ipc().set_value(address);
}

void Cpu::ClearIpcForTest() {
  ipc_valid_ = false;
}

void Cpu::SetCurrentPhaseForTest(base::TickPhase phase) {
  current_phase_ = phase;
}

void Cpu::TickProcess() {
  // First propagate to all children
  Component::TickProcess();

  // Then do CPU-specific processing
  if (halt_control_.asserted()) {
    halted_ = true;
  }
  if (crash_control_.asserted()) {
    crashed_ = true;
    halted_ = true;
  }
  if (controller_.ipc().latch().asserted()) {
    ipc_valid_ = true;
    if (trace_.enabled()) {
      DebugTraceEntry entry;
      entry.cycle = cycle_count_;
      entry.instruction_address = controller_.ipc().value();
      entry.pc = pc_.value();
      entry.ir = controller_.ir().value();
      entry.sc = controller_.sc().value();
      entry.a = a_.value();
      entry.x = x_.value();
      entry.status = status_.value();
      trace_.Record(std::move(entry));
    }
  }
}

}  // namespace irata2::sim

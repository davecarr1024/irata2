#include "irata2/sim/debug_dump.h"

#include "irata2/base/types.h"
#include "irata2/sim/cpu.h"

#include <iomanip>
#include <sstream>

namespace irata2::sim {

namespace {
std::string HexByte(base::Byte value) {
  std::ostringstream out;
  out << "0x" << std::hex << std::setw(2) << std::setfill('0')
      << static_cast<int>(value.value()) << std::dec;
  return out.str();
}

std::string HexWord(base::Word value) {
  std::ostringstream out;
  out << "0x" << std::hex << std::setw(4) << std::setfill('0')
      << value.value() << std::dec;
  return out.str();
}

std::string FormatLocation(const std::optional<SourceLocation>& location) {
  if (!location) {
    return "unknown";
  }
  std::ostringstream out;
  out << location->file << ":" << location->line << ":" << location->column;
  if (!location->text.empty()) {
    out << " " << location->text;
  }
  return out.str();
}

std::string FormatFlags(const StatusRegister& status) {
  std::ostringstream out;
  out << "N=" << (status.negative().value() ? '1' : '0') << " ";
  out << "V=" << (status.overflow().value() ? '1' : '0') << " ";
  out << "U=" << (status.unused().value() ? '1' : '0') << " ";
  out << "B=" << (status.brk().value() ? '1' : '0') << " ";
  out << "D=" << (status.decimal().value() ? '1' : '0') << " ";
  out << "I=" << (status.interrupt_disable().value() ? '1' : '0') << " ";
  out << "Z=" << (status.zero().value() ? '1' : '0') << " ";
  out << "C=" << (status.carry().value() ? '1' : '0');
  return out.str();
}

std::string FormatBusValue(const ByteBus& bus) {
  if (!bus.has_value()) {
    return "--";
  }
  return HexByte(bus.value());
}

std::string FormatBusValue(const WordBus& bus) {
  if (!bus.has_value()) {
    return "--";
  }
  return HexWord(bus.value());
}

std::string FormatTraceLocation(const DebugSymbols* symbols,
                                base::Word address) {
  if (!symbols) {
    return "unknown";
  }
  return FormatLocation(symbols->Lookup(address));
}
}  // namespace

std::string FormatDebugDump(const Cpu& cpu, std::string_view reason) {
  std::ostringstream out;
  const auto* symbols = cpu.debug_symbols();
  const auto instruction_address = cpu.instruction_address();

  out << "Debug dump (" << reason << ")\n";
  out << "cycle: " << cpu.cycle_count() << "\n";
  out << "instruction: " << HexWord(instruction_address) << " "
      << FormatLocation(cpu.instruction_source_location()) << "\n";
  out << "pc: " << HexWord(cpu.pc().value())
      << " ipc: " << HexWord(cpu.controller().ipc().value())
      << " ir: " << HexByte(cpu.controller().ir().value())
      << " sc: " << HexByte(cpu.controller().sc().value()) << "\n";
  out << "a: " << HexByte(cpu.a().value())
      << " x: " << HexByte(cpu.x().value())
      << " sr: " << HexByte(cpu.status().value())
      << " flags: " << FormatFlags(cpu.status()) << "\n";
  out << "buses: data=" << FormatBusValue(cpu.data_bus())
      << " address=" << FormatBusValue(cpu.address_bus()) << "\n";

  const auto trace_entries = cpu.trace_entries();
  out << "trace (" << trace_entries.size() << " entries):\n";
  for (size_t i = 0; i < trace_entries.size(); ++i) {
    const auto& entry = trace_entries[i];
    out << "  [" << i << "] cycle=" << entry.cycle
        << " addr=" << HexWord(entry.instruction_address)
        << " ir=" << HexByte(entry.ir)
        << " pc=" << HexWord(entry.pc)
        << " sc=" << HexByte(entry.sc)
        << " a=" << HexByte(entry.a)
        << " x=" << HexByte(entry.x)
        << " sr=" << HexByte(entry.status)
        << " " << FormatTraceLocation(symbols, entry.instruction_address)
        << "\n";
  }

  return out.str();
}

}  // namespace irata2::sim

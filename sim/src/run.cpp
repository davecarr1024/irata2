#include "irata2/sim.h"
#include "irata2/sim/debug_dump.h"
#include "irata2/base/log.h"

#include <cstdint>
#include <iostream>

namespace {
void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " [--expect-crash] [--max-cycles N] [--debug debug.json]"
            << " [--trace-depth N]"
            << " <cartridge.bin>\n";
}
}  // namespace

int main(int argc, char** argv) {
  irata2::base::InitializeLogging();

  if (argc < 2) {
    PrintUsage(argv[0]);
    return 1;
  }

  bool expect_crash = false;
  int64_t max_cycles = -1;
  int64_t trace_depth = -1;
  std::string debug_path;
  std::string cartridge_path;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--expect-crash") {
      expect_crash = true;
      continue;
    }
    if (arg == "--max-cycles") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      max_cycles = std::stoll(argv[++i]);
      continue;
    }
    if (arg == "--debug") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      debug_path = argv[++i];
      continue;
    }
    if (arg == "--trace-depth") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      trace_depth = std::stoll(argv[++i]);
      continue;
    }
    if (cartridge_path.empty()) {
      cartridge_path = std::move(arg);
      continue;
    }
    PrintUsage(argv[0]);
    return 1;
  }

  if (cartridge_path.empty()) {
    PrintUsage(argv[0]);
    return 1;
  }

  try {
    irata2::sim::LoadedCartridge cartridge =
        irata2::sim::LoadCartridge(cartridge_path);
    auto rom = irata2::sim::memory::MakeRom(std::move(cartridge.rom));

    irata2::sim::Cpu cpu(irata2::sim::DefaultHdl(),
                         irata2::sim::DefaultMicrocodeProgram(),
                         std::move(rom));
    cpu.pc().set_value(cartridge.header.entry);
    cpu.controller().sc().set_value(irata2::base::Byte{0});
    cpu.controller().ir().set_value(cpu.memory().ReadAt(cartridge.header.entry));
    if (!debug_path.empty()) {
      cpu.LoadDebugSymbols(irata2::sim::LoadDebugSymbols(debug_path));
      const int64_t depth = trace_depth >= 0 ? trace_depth : 64;
      cpu.EnableTrace(static_cast<size_t>(depth));
    } else if (trace_depth >= 0) {
      cpu.EnableTrace(static_cast<size_t>(trace_depth));
    }

    // Log sim.start
    IRATA2_LOG_INFO << "sim.start: cartridge=" << cartridge_path
                    << ", entry_pc=" << cartridge.header.entry.to_string()
                    << ", trace_depth=" << (trace_depth >= 0 ? trace_depth : (debug_path.empty() ? 0 : 64))
                    << ", debug_symbols=" << (!debug_path.empty() ? debug_path : "none");

    irata2::sim::Cpu::RunResult result;
    bool timed_out = false;
    if (max_cycles < 0) {
      result = cpu.RunUntilHalt();
    } else {
      uint64_t remaining = static_cast<uint64_t>(max_cycles);
      while (!cpu.halted() && remaining > 0) {
        cpu.Tick();
        --remaining;
      }
      result.halted = cpu.halted();
      result.crashed = cpu.crashed();
      if (!result.halted) {
        timed_out = true;
      }
    }

    // Log lifecycle events
    if (timed_out) {
      IRATA2_LOG_INFO << "sim.timeout: max_cycles=" << max_cycles
                      << ", cycle_count=" << cpu.cycle_count()
                      << ", instruction_address=" << cpu.instruction_address().to_string();
    } else if (result.crashed) {
      IRATA2_LOG_INFO << "sim.crash: cycle_count=" << cpu.cycle_count()
                      << ", instruction_address=" << cpu.instruction_address().to_string();
    } else {
      IRATA2_LOG_INFO << "sim.halt: cycle_count=" << cpu.cycle_count()
                      << ", instruction_address=" << cpu.instruction_address().to_string();
    }

    // Log failure-path debug dump and trace
    if (!debug_path.empty()) {
      const bool unexpected_crash = result.crashed && !expect_crash;
      const bool unexpected_halt = !result.crashed && expect_crash;
      if (timed_out || unexpected_crash || unexpected_halt) {
        const std::string reason = timed_out ? "timeout"
                                  : unexpected_crash ? "crash"
                                                     : "halt";
        const std::string dump = irata2::sim::FormatDebugDump(cpu, reason);
        IRATA2_LOG_INFO << "sim.dump:\n" << dump;
        std::cerr << dump << "\n";
      }
    }

    if (timed_out) {
      return 4;
    }
    if (expect_crash) {
      return result.crashed ? 0 : 2;
    }
    return result.crashed ? 2 : 0;
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    return 1;
  }
}

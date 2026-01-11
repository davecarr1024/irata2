#include "irata2/sim.h"

#include <cstdint>
#include <iostream>

namespace {
void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " [--expect-crash] [--max-cycles N] [--debug debug.json]"
            << " <cartridge.bin>\n";
}
}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage(argv[0]);
    return 1;
  }

  bool expect_crash = false;
  int64_t max_cycles = -1;
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
    }

    irata2::sim::Cpu::RunResult result;
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
        return 4;
      }
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

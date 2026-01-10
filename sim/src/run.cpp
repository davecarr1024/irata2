#include "irata2/sim.h"

#include <iostream>

namespace {
void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " <cartridge.bin>\n";
}
}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage(argv[0]);
    return 1;
  }

  const std::string cartridge_path = argv[1];

  try {
    irata2::sim::LoadedCartridge cartridge =
        irata2::sim::LoadCartridge(cartridge_path);
    auto rom = irata2::sim::memory::MakeRom(std::move(cartridge.rom));

    irata2::sim::Cpu cpu(irata2::sim::DefaultHdl(),
                         irata2::sim::DefaultMicrocodeProgram(),
                         std::move(rom));
    cpu.pc().set_value(cartridge.header.entry);

    const auto result = cpu.RunUntilHalt();
    if (result.crashed) {
      return 2;
    }
    return result.halted ? 0 : 3;
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    return 1;
  }
}

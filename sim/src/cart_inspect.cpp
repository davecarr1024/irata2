#include "irata2/sim/cartridge.h"

#include <iostream>
#include <optional>
#include <string>

#include "irata2/sim/debug_symbols.h"

namespace {
void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " --rom <cartridge.bin> [--debug <debug.json>]\n";
}
}  // namespace

int main(int argc, char** argv) {
  std::string rom_path;
  std::string debug_path;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--rom") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      rom_path = argv[++i];
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
    PrintUsage(argv[0]);
    return 1;
  }

  if (rom_path.empty()) {
    PrintUsage(argv[0]);
    return 1;
  }

  try {
    auto cartridge = irata2::sim::LoadCartridge(rom_path);
    const auto& header = cartridge.header;

    std::cout << "cartridge:\n";
    std::cout << "  magic=" << header.magic[0] << header.magic[1]
              << header.magic[2] << header.magic[3] << "\n";
    std::cout << "  version=" << header.version << "\n";
    std::cout << "  header_size=" << header.header_size << "\n";
    std::cout << "  entry=" << header.entry.to_string() << "\n";
    std::cout << "  rom_size=" << header.rom_size << "\n";
    std::cout << "  rom_bytes=" << cartridge.rom.size() << "\n";

    if (!debug_path.empty()) {
      auto symbols = irata2::sim::LoadDebugSymbols(debug_path);
      std::cout << "debug:\n";
      std::cout << "  version=" << symbols.version << "\n";
      std::cout << "  entry=" << symbols.entry.to_string() << "\n";
      std::cout << "  rom_size=" << symbols.rom_size << "\n";
      std::cout << "  symbols=" << symbols.symbols.size() << "\n";
      std::cout << "  records=" << symbols.records.size() << "\n";
      std::cout << "  sources=" << symbols.source_files.size() << "\n";
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    return 1;
  }
}

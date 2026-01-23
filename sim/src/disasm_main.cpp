#include "irata2/sim/disassembler.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

#include "irata2/sim/cartridge.h"

namespace {
void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " --rom <cartridge.bin>"
            << " [--debug <debug.json>]"
            << " [--origin <hex>]"
            << " [--show-addresses] [--show-bytes]"
            << " [--no-labels] [--no-org]\n";
}

std::optional<uint16_t> ParseU16(const std::string& value) {
  try {
    std::string text = value;
    int base = 10;
    if (!text.empty() && text.front() == '$') {
      text.erase(text.begin());
      base = 16;
    }
    size_t idx = 0;
    unsigned long parsed = std::stoul(text, &idx, base);
    if (idx != text.size() || parsed > 0xFFFF) {
      return std::nullopt;
    }
    return static_cast<uint16_t>(parsed);
  } catch (const std::exception&) {
    return std::nullopt;
  }
}
}  // namespace

int main(int argc, char** argv) {
  std::string rom_path;
  std::string debug_path;
  irata2::sim::DisassembleOptions options;

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
    if (arg == "--origin") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      auto parsed = ParseU16(argv[++i]);
      if (!parsed) {
        std::cerr << "Invalid origin value\n";
        return 1;
      }
      options.origin = irata2::base::Word{*parsed};
      continue;
    }
    if (arg == "--show-addresses") {
      options.emit_addresses = true;
      continue;
    }
    if (arg == "--show-bytes") {
      options.emit_bytes = true;
      continue;
    }
    if (arg == "--no-labels") {
      options.emit_labels = false;
      continue;
    }
    if (arg == "--no-org") {
      options.emit_org = false;
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
    std::optional<irata2::sim::DebugSymbols> symbols;
    if (!debug_path.empty()) {
      symbols = irata2::sim::LoadDebugSymbols(debug_path);
    }
    std::string output = irata2::sim::Disassemble(
        cartridge.rom,
        symbols ? &*symbols : nullptr,
        options);
    std::cout << output;
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    return 1;
  }
}

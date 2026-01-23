#include "irata2/frontend/demo_runner.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

namespace {
void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " --rom <cartridge.bin>"
            << " [--fps N] [--scale N] [--cycles-per-frame N]"
            << " [--debug-on-crash] [--trace-size N]\n";
}

std::optional<int64_t> ParseI64(const std::string& value) {
  try {
    size_t idx = 0;
    int64_t parsed = std::stoll(value, &idx);
    if (idx != value.size()) {
      return std::nullopt;
    }
    return parsed;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}
}  // namespace

int main(int argc, char** argv) {
  irata2::frontend::DemoOptions options;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--rom") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      options.rom_path = argv[++i];
      continue;
    }
    if (arg == "--fps") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      auto parsed = ParseI64(argv[++i]);
      if (!parsed || *parsed <= 0) {
        std::cerr << "Invalid fps value\n";
        return 1;
      }
      options.fps = static_cast<int>(*parsed);
      continue;
    }
    if (arg == "--scale") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      auto parsed = ParseI64(argv[++i]);
      if (!parsed || *parsed <= 0) {
        std::cerr << "Invalid scale value\n";
        return 1;
      }
      options.scale = static_cast<int>(*parsed);
      continue;
    }
    if (arg == "--cycles-per-frame") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      auto parsed = ParseI64(argv[++i]);
      if (!parsed || *parsed <= 0) {
        std::cerr << "Invalid cycles per frame value\n";
        return 1;
      }
      options.cycles_per_frame = *parsed;
      continue;
    }
    if (arg == "--debug-on-crash") {
      options.debug_on_crash = true;
      continue;
    }
    if (arg == "--trace-size") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      auto parsed = ParseI64(argv[++i]);
      if (!parsed || *parsed < 0) {
        std::cerr << "Invalid trace size value\n";
        return 1;
      }
      options.trace_size = static_cast<size_t>(*parsed);
      continue;
    }
    PrintUsage(argv[0]);
    return 1;
  }

  if (options.rom_path.empty()) {
    PrintUsage(argv[0]);
    return 1;
  }

  try {
    irata2::frontend::DemoRunner runner(options);
    return runner.Run();
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    return 1;
  }
}

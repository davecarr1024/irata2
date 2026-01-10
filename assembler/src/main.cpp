#include "irata2/assembler/assembler.h"

#include <iostream>

#include "irata2/assembler/error.h"

namespace {
void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " <input.asm> <output.bin> [output.json]\n";
}
}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    PrintUsage(argv[0]);
    return 1;
  }

  const std::string input = argv[1];
  const std::string output = argv[2];
  const std::string debug = argc >= 4 ? argv[3] : "";

  try {
    irata2::assembler::AssemblerResult result =
        irata2::assembler::AssembleFile(input);
    irata2::assembler::WriteCartridge(result, output);
    if (!debug.empty()) {
      irata2::assembler::WriteDebugInfo(result, debug);
    }
  } catch (const irata2::assembler::AssemblerError& error) {
    std::cerr << "Assembler error";
    if (!error.span().file.empty()) {
      std::cerr << " at " << error.span().file << ":" << error.span().line
                << ":" << error.span().column;
    }
    std::cerr << ": " << error.what() << "\n";
    return 1;
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    return 1;
  }

  return 0;
}

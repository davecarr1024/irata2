#ifndef IRATA2_ASSEMBLER_ASSEMBLER_H
#define IRATA2_ASSEMBLER_ASSEMBLER_H

#include <string>
#include <string_view>
#include <vector>

#include "irata2/assembler/cartridge.h"
#include "irata2/base/types.h"

namespace irata2::assembler {

struct AssemblerOptions {
  base::Word origin = base::Word{0x8000};
  uint32_t rom_size = 0x8000;
};

struct AssemblerResult {
  CartridgeHeader header;
  std::vector<uint8_t> rom;
  std::string debug_json;
};

AssemblerResult Assemble(std::string_view source,
                         const std::string& filename,
                         const AssemblerOptions& options = {});

AssemblerResult AssembleFile(const std::string& path,
                             const AssemblerOptions& options = {});

void WriteCartridge(const AssemblerResult& result, const std::string& path);
void WriteDebugInfo(const AssemblerResult& result, const std::string& path);

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_ASSEMBLER_H

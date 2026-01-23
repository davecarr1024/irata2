#ifndef IRATA2_SIM_DISASSEMBLER_H
#define IRATA2_SIM_DISASSEMBLER_H

#include <string>
#include <vector>

#include "irata2/base/types.h"
#include "irata2/sim/debug_symbols.h"

namespace irata2::sim {

struct DisassembleOptions {
  base::Word origin = base::Word{0x8000};
  bool emit_org = true;
  bool emit_labels = true;
  bool emit_addresses = false;
  bool emit_bytes = false;
};

std::string Disassemble(const std::vector<base::Byte>& rom,
                        const DebugSymbols* symbols = nullptr,
                        const DisassembleOptions& options = {});

}  // namespace irata2::sim

#endif  // IRATA2_SIM_DISASSEMBLER_H

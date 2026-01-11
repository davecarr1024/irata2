#include "irata2/sim/cpu.h"

namespace irata2::sim {

void Cpu::LoadDebugSymbols(DebugSymbols symbols) {
  debug_symbols_ = std::move(symbols);
}

const DebugSymbols* Cpu::debug_symbols() const {
  if (!debug_symbols_) {
    return nullptr;
  }
  return &*debug_symbols_;
}

}  // namespace irata2::sim

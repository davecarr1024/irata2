#ifndef IRATA2_SIM_DEBUG_SYMBOLS_H
#define IRATA2_SIM_DEBUG_SYMBOLS_H

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "irata2/base/types.h"

namespace irata2::sim {

struct SourceLocation {
  std::string file;
  int line = 1;
  int column = 1;
  std::string text;
};

struct DebugRecord {
  base::Word address;
  uint32_t rom_offset = 0;
  SourceLocation location;
};

struct DebugSymbols {
  std::string version;
  base::Word entry;
  uint32_t rom_size = 0;
  uint32_t cartridge_version = 0;
  std::string source_root;
  std::vector<std::string> source_files;
  std::unordered_map<std::string, base::Word> symbols;
  std::unordered_map<uint16_t, SourceLocation> pc_to_source;
  std::vector<DebugRecord> records;

  std::optional<SourceLocation> Lookup(base::Word address) const;
};

DebugSymbols LoadDebugSymbols(const std::string& path);

}  // namespace irata2::sim

#endif  // IRATA2_SIM_DEBUG_SYMBOLS_H

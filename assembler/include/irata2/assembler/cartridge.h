#ifndef IRATA2_ASSEMBLER_CARTRIDGE_H
#define IRATA2_ASSEMBLER_CARTRIDGE_H

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "irata2/base/types.h"

namespace irata2::assembler {

constexpr uint16_t kCartridgeHeaderSize = 32;
constexpr uint16_t kCartridgeVersion = 1;

struct CartridgeHeader {
  std::array<char, 4> magic{{'I', 'R', 'T', 'A'}};
  uint16_t version = kCartridgeVersion;
  uint16_t header_size = kCartridgeHeaderSize;
  base::Word entry = base::Word{0x8000};
  uint32_t rom_size = 0;
  std::array<uint8_t, 18> reserved{};
};

std::vector<uint8_t> EncodeHeader(const CartridgeHeader& header);

struct DebugRecord {
  base::Word address;
  uint32_t rom_offset = 0;
  std::string file;
  int line = 1;
  int column = 1;
  std::string text;
};

std::string EncodeDebugJson(const CartridgeHeader& header,
                            std::string_view schema_version,
                            std::string_view source_root,
                            const std::vector<std::string>& source_files,
                            const std::unordered_map<std::string, base::Word>& symbols,
                            const std::vector<DebugRecord>& records);

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_CARTRIDGE_H

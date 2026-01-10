#ifndef IRATA2_ASSEMBLER_CARTRIDGE_H
#define IRATA2_ASSEMBLER_CARTRIDGE_H

#include <array>
#include <cstdint>
#include <string>
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

std::string EncodeDebugJson(const CartridgeHeader& header,
                            const std::vector<std::string>& lines,
                            const std::vector<base::Word>& addresses,
                            const std::vector<uint32_t>& rom_offsets,
                            const std::vector<int>& line_numbers,
                            const std::vector<int>& column_numbers);

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_CARTRIDGE_H

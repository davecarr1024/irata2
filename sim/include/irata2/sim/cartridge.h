#ifndef IRATA2_SIM_CARTRIDGE_H
#define IRATA2_SIM_CARTRIDGE_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "irata2/base/types.h"
#include "irata2/sim/memory/module.h"

namespace irata2::sim {

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

struct LoadedCartridge {
  CartridgeHeader header;
  std::vector<base::Byte> rom;
};

LoadedCartridge LoadCartridge(const std::string& path);
std::shared_ptr<memory::Module> LoadCartridgeRom(const std::string& path);

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CARTRIDGE_H

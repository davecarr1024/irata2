#include "irata2/sim/cartridge.h"

#include <fstream>
#include <sstream>

#include "irata2/sim/error.h"

namespace irata2::sim {

namespace {
uint16_t ReadU16(const std::vector<uint8_t>& data, size_t offset) {
  if (offset + 1 >= data.size()) {
    throw SimError("cartridge header truncated");
  }
  return static_cast<uint16_t>(data[offset]) |
         (static_cast<uint16_t>(data[offset + 1]) << 8);
}

uint32_t ReadU32(const std::vector<uint8_t>& data, size_t offset) {
  if (offset + 3 >= data.size()) {
    throw SimError("cartridge header truncated");
  }
  return static_cast<uint32_t>(data[offset]) |
         (static_cast<uint32_t>(data[offset + 1]) << 8) |
         (static_cast<uint32_t>(data[offset + 2]) << 16) |
         (static_cast<uint32_t>(data[offset + 3]) << 24);
}

std::vector<uint8_t> ReadFile(const std::string& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    throw SimError("failed to open cartridge: " + path);
  }
  std::vector<uint8_t> data;
  input.seekg(0, std::ios::end);
  std::streamsize size = input.tellg();
  input.seekg(0, std::ios::beg);
  if (size <= 0) {
    throw SimError("cartridge file is empty: " + path);
  }
  data.resize(static_cast<size_t>(size));
  input.read(reinterpret_cast<char*>(data.data()), size);
  return data;
}
}  // namespace

LoadedCartridge LoadCartridge(const std::string& path) {
  const std::vector<uint8_t> data = ReadFile(path);
  if (data.size() < kCartridgeHeaderSize) {
    throw SimError("cartridge header too small");
  }

  CartridgeHeader header;
  for (size_t i = 0; i < header.magic.size(); ++i) {
    header.magic[i] = static_cast<char>(data[i]);
  }
  if (header.magic != std::array<char, 4>{{'I', 'R', 'T', 'A'}}) {
    throw SimError("cartridge magic mismatch");
  }

  header.version = ReadU16(data, 4);
  header.header_size = ReadU16(data, 6);
  header.entry = base::Word{ReadU16(data, 8)};
  header.rom_size = ReadU32(data, 10);

  if (header.header_size < kCartridgeHeaderSize) {
    throw SimError("cartridge header size too small");
  }
  if (data.size() < static_cast<size_t>(header.header_size + header.rom_size)) {
    throw SimError("cartridge file truncated");
  }

  LoadedCartridge result;
  result.header = header;
  result.rom.reserve(header.rom_size);
  size_t start = header.header_size;
  for (size_t i = 0; i < header.rom_size; ++i) {
    result.rom.push_back(base::Byte{data[start + i]});
  }
  return result;
}

std::vector<base::Byte> LoadCartridgeRom(const std::string& path) {
  LoadedCartridge cartridge = LoadCartridge(path);
  return std::move(cartridge.rom);
}

}  // namespace irata2::sim

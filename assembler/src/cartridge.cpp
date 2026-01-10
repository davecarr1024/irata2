#include "irata2/assembler/cartridge.h"

#include <sstream>

namespace irata2::assembler {

namespace {
void AppendU16(std::vector<uint8_t>& out, uint16_t value) {
  out.push_back(static_cast<uint8_t>(value & 0xFF));
  out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void AppendU32(std::vector<uint8_t>& out, uint32_t value) {
  out.push_back(static_cast<uint8_t>(value & 0xFF));
  out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
  out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

std::string EscapeJson(const std::string& value) {
  std::ostringstream out;
  for (char ch : value) {
    switch (ch) {
      case '"': out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      default:
        if (static_cast<unsigned char>(ch) < 0x20) {
          out << "\\u";
          out << std::hex;
          out.width(4);
          out.fill('0');
          out << static_cast<int>(static_cast<unsigned char>(ch));
          out << std::dec;
        } else {
          out << ch;
        }
        break;
    }
  }
  return out.str();
}
}  // namespace

std::vector<uint8_t> EncodeHeader(const CartridgeHeader& header) {
  std::vector<uint8_t> out;
  out.reserve(kCartridgeHeaderSize);

  for (char ch : header.magic) {
    out.push_back(static_cast<uint8_t>(ch));
  }
  AppendU16(out, header.version);
  AppendU16(out, header.header_size);
  AppendU16(out, static_cast<uint16_t>(header.entry.value()));
  AppendU32(out, header.rom_size);
  for (uint8_t byte : header.reserved) {
    out.push_back(byte);
  }

  while (out.size() < kCartridgeHeaderSize) {
    out.push_back(0);
  }

  return out;
}

std::string EncodeDebugJson(const CartridgeHeader& header,
                            const std::vector<std::string>& lines,
                            const std::vector<base::Word>& addresses,
                            const std::vector<uint32_t>& rom_offsets,
                            const std::vector<int>& line_numbers,
                            const std::vector<int>& column_numbers) {
  std::ostringstream out;
  out << "{\n";
  out << "  \"version\": " << header.version << ",\n";
  out << "  \"entry\": \"0x" << std::hex << header.entry.value() << std::dec << "\",\n";
  out << "  \"rom_size\": " << header.rom_size << ",\n";
  out << "  \"records\": [\n";

  for (size_t i = 0; i < lines.size(); ++i) {
    out << "    {";
    out << "\"address\": \"0x" << std::hex << addresses[i].value() << std::dec << "\", ";
    out << "\"rom_offset\": " << rom_offsets[i] << ", ";
    out << "\"line\": " << line_numbers[i] << ", ";
    out << "\"column\": " << column_numbers[i] << ", ";
    out << "\"text\": \"" << EscapeJson(lines[i]) << "\"";
    out << "}";
    if (i + 1 < lines.size()) {
      out << ",";
    }
    out << "\n";
  }

  out << "  ]\n";
  out << "}\n";
  return out.str();
}

}  // namespace irata2::assembler

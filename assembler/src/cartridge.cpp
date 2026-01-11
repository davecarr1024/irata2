#include "irata2/assembler/cartridge.h"

#include <map>
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
                            std::string_view schema_version,
                            std::string_view source_root,
                            const std::vector<std::string>& source_files,
                            const std::unordered_map<std::string, base::Word>& symbols,
                            const std::vector<DebugRecord>& records) {
  std::ostringstream out;
  out << "{\n";
  out << "  \"version\": \"" << EscapeJson(std::string(schema_version)) << "\",\n";
  out << "  \"cartridge_version\": " << header.version << ",\n";
  out << "  \"entry\": \"0x" << std::hex << header.entry.value() << std::dec << "\",\n";
  out << "  \"rom_size\": " << header.rom_size << ",\n";
  out << "  \"source_root\": \"" << EscapeJson(std::string(source_root)) << "\",\n";
  out << "  \"source_files\": [";
  for (size_t i = 0; i < source_files.size(); ++i) {
    out << "\"" << EscapeJson(source_files[i]) << "\"";
    if (i + 1 < source_files.size()) {
      out << ", ";
    }
  }
  out << "],\n";

  out << "  \"symbols\": {";
  std::map<std::string, base::Word> sorted_symbols(symbols.begin(), symbols.end());
  if (!sorted_symbols.empty()) {
    out << "\n";
    size_t count = 0;
    for (const auto& [name, address] : sorted_symbols) {
      out << "    \"" << EscapeJson(name) << "\": \"0x" << std::hex << address.value()
          << std::dec << "\"";
      if (++count < sorted_symbols.size()) {
        out << ",";
      }
      out << "\n";
    }
    out << "  ";
  }
  out << "},\n";

  out << "  \"pc_to_source\": {\n";
  std::map<uint16_t, DebugRecord> pc_entries;
  for (const auto& record : records) {
    pc_entries.emplace(record.address.value(), record);
  }
  size_t pc_index = 0;
  for (const auto& [address, record] : pc_entries) {
    out << "    \"0x" << std::hex << address << std::dec << "\": {";
    out << "\"file\": \"" << EscapeJson(record.file) << "\", ";
    out << "\"line\": " << record.line << ", ";
    out << "\"column\": " << record.column << ", ";
    out << "\"text\": \"" << EscapeJson(record.text) << "\"";
    out << "}";
    if (++pc_index < pc_entries.size()) {
      out << ",";
    }
    out << "\n";
  }
  out << "  },\n";

  out << "  \"records\": [\n";

  for (size_t i = 0; i < records.size(); ++i) {
    const auto& record = records[i];
    out << "    {";
    out << "\"address\": \"0x" << std::hex << record.address.value() << std::dec << "\", ";
    out << "\"rom_offset\": " << record.rom_offset << ", ";
    out << "\"file\": \"" << EscapeJson(record.file) << "\", ";
    out << "\"line\": " << record.line << ", ";
    out << "\"column\": " << record.column << ", ";
    out << "\"text\": \"" << EscapeJson(record.text) << "\"";
    out << "}";
    if (i + 1 < records.size()) {
      out << ",";
    }
    out << "\n";
  }

  out << "  ]\n";
  out << "}\n";
  return out.str();
}

}  // namespace irata2::assembler

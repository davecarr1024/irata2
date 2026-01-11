#include "irata2/sim/debug_symbols.h"

#include "irata2/sim/error.h"

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>

namespace irata2::sim {

namespace {
struct JsonValue {
  enum class Type { Object, Array, String, Number, Bool, Null };

  Type type = Type::Null;
  std::map<std::string, JsonValue> object;
  std::vector<JsonValue> array;
  std::string string;
  int64_t number = 0;
  bool boolean = false;
};

class JsonParser {
 public:
  explicit JsonParser(std::string_view input) : input_(input) {}

  JsonValue Parse() {
    SkipWhitespace();
    JsonValue value = ParseValue();
    SkipWhitespace();
    if (pos_ != input_.size()) {
      throw SimError("extra data after JSON document");
    }
    return value;
  }

 private:
  JsonValue ParseValue() {
    SkipWhitespace();
    if (Match("null")) {
      return JsonValue{};
    }
    if (Match("true")) {
      JsonValue value;
      value.type = JsonValue::Type::Bool;
      value.boolean = true;
      return value;
    }
    if (Match("false")) {
      JsonValue value;
      value.type = JsonValue::Type::Bool;
      value.boolean = false;
      return value;
    }
    char ch = Peek();
    if (ch == '{') {
      return ParseObject();
    }
    if (ch == '[') {
      return ParseArray();
    }
    if (ch == '"') {
      JsonValue value;
      value.type = JsonValue::Type::String;
      value.string = ParseString();
      return value;
    }
    if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) {
      JsonValue value;
      value.type = JsonValue::Type::Number;
      value.number = ParseNumber();
      return value;
    }
    throw SimError("unexpected character in JSON");
  }

  JsonValue ParseObject() {
    Consume('{');
    JsonValue value;
    value.type = JsonValue::Type::Object;
    SkipWhitespace();
    if (Peek() == '}') {
      Consume('}');
      return value;
    }
    while (true) {
      SkipWhitespace();
      std::string key = ParseString();
      SkipWhitespace();
      Consume(':');
      value.object.emplace(std::move(key), ParseValue());
      SkipWhitespace();
      char ch = Peek();
      if (ch == '}') {
        Consume('}');
        break;
      }
      Consume(',');
    }
    return value;
  }

  JsonValue ParseArray() {
    Consume('[');
    JsonValue value;
    value.type = JsonValue::Type::Array;
    SkipWhitespace();
    if (Peek() == ']') {
      Consume(']');
      return value;
    }
    while (true) {
      value.array.push_back(ParseValue());
      SkipWhitespace();
      char ch = Peek();
      if (ch == ']') {
        Consume(']');
        break;
      }
      Consume(',');
    }
    return value;
  }

  std::string ParseString() {
    Consume('"');
    std::ostringstream out;
    while (pos_ < input_.size()) {
      char ch = input_[pos_++];
      if (ch == '"') {
        return out.str();
      }
      if (ch == '\\') {
        if (pos_ >= input_.size()) {
          throw SimError("unterminated escape sequence in JSON string");
        }
        char esc = input_[pos_++];
        switch (esc) {
          case '"': out << '"'; break;
          case '\\': out << '\\'; break;
          case '/': out << '/'; break;
          case 'b': out << '\b'; break;
          case 'f': out << '\f'; break;
          case 'n': out << '\n'; break;
          case 'r': out << '\r'; break;
          case 't': out << '\t'; break;
          case 'u': out << ParseUnicodeEscape(); break;
          default:
            throw SimError("invalid escape sequence in JSON string");
        }
        continue;
      }
      out << ch;
    }
    throw SimError("unterminated JSON string");
  }

  char ParseUnicodeEscape() {
    if (pos_ + 4 > input_.size()) {
      throw SimError("truncated unicode escape in JSON string");
    }
    int value = 0;
    for (int i = 0; i < 4; ++i) {
      char ch = input_[pos_++];
      int digit = 0;
      if (ch >= '0' && ch <= '9') {
        digit = ch - '0';
      } else if (ch >= 'a' && ch <= 'f') {
        digit = 10 + (ch - 'a');
      } else if (ch >= 'A' && ch <= 'F') {
        digit = 10 + (ch - 'A');
      } else {
        throw SimError("invalid unicode escape in JSON string");
      }
      value = (value << 4) + digit;
    }
    if (value < 0x80) {
      return static_cast<char>(value);
    }
    return '?';
  }

  int64_t ParseNumber() {
    size_t start = pos_;
    if (Peek() == '-') {
      ++pos_;
    }
    while (pos_ < input_.size() &&
           std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
      ++pos_;
    }
    std::string_view token = input_.substr(start, pos_ - start);
    if (token.empty() || token == "-") {
      throw SimError("invalid number in JSON");
    }
    return std::stoll(std::string(token));
  }

  void SkipWhitespace() {
    while (pos_ < input_.size()) {
      char ch = input_[pos_];
      if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        ++pos_;
        continue;
      }
      break;
    }
  }

  char Peek() const {
    if (pos_ >= input_.size()) {
      return '\0';
    }
    return input_[pos_];
  }

  void Consume(char expected) {
    if (Peek() != expected) {
      throw SimError("unexpected JSON token");
    }
    ++pos_;
  }

  bool Match(std::string_view token) {
    if (input_.substr(pos_, token.size()) != token) {
      return false;
    }
    pos_ += token.size();
    return true;
  }

  std::string_view input_;
  size_t pos_ = 0;
};

const JsonValue& RequireObject(const JsonValue& value) {
  if (value.type != JsonValue::Type::Object) {
    throw SimError("expected JSON object");
  }
  return value;
}

const JsonValue& RequireField(const JsonValue& object, std::string_view key) {
  const auto& obj = RequireObject(object).object;
  auto it = obj.find(std::string(key));
  if (it == obj.end()) {
    throw SimError("missing debug symbol field: " + std::string(key));
  }
  return it->second;
}

std::string RequireString(const JsonValue& value, std::string_view label) {
  if (value.type != JsonValue::Type::String) {
    throw SimError("expected string for " + std::string(label));
  }
  return value.string;
}

int64_t RequireNumber(const JsonValue& value, std::string_view label) {
  if (value.type != JsonValue::Type::Number) {
    throw SimError("expected number for " + std::string(label));
  }
  return value.number;
}

const JsonValue& RequireArray(const JsonValue& value, std::string_view label) {
  if (value.type != JsonValue::Type::Array) {
    throw SimError("expected array for " + std::string(label));
  }
  return value;
}

base::Word ParseHexWord(const std::string& value, std::string_view label) {
  if (value.rfind("0x", 0) != 0 && value.rfind("0X", 0) != 0) {
    throw SimError("expected hex value for " + std::string(label));
  }
  uint32_t parsed = 0;
  try {
    parsed = static_cast<uint32_t>(std::stoul(value.substr(2), nullptr, 16));
  } catch (const std::exception&) {
    throw SimError("invalid hex value for " + std::string(label));
  }
  if (parsed > 0xFFFFu) {
    throw SimError("hex value out of range for " + std::string(label));
  }
  return base::Word{static_cast<uint16_t>(parsed)};
}

std::vector<std::string> ParseStringArray(const JsonValue& value,
                                          std::string_view label) {
  const auto& array = RequireArray(value, label);
  std::vector<std::string> out;
  out.reserve(array.array.size());
  for (const auto& item : array.array) {
    out.push_back(RequireString(item, label));
  }
  return out;
}

std::string ReadFile(const std::string& path) {
  std::ifstream input(path);
  if (!input) {
    throw SimError("failed to open debug symbols: " + path);
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}
}  // namespace

std::optional<SourceLocation> DebugSymbols::Lookup(base::Word address) const {
  auto it = pc_to_source.find(address.value());
  if (it == pc_to_source.end()) {
    return std::nullopt;
  }
  return it->second;
}

DebugSymbols LoadDebugSymbols(const std::string& path) {
  const std::string content = ReadFile(path);
  JsonParser parser(content);
  JsonValue root = parser.Parse();
  const auto& obj = RequireObject(root).object;

  DebugSymbols symbols;
  symbols.version = RequireString(RequireField(root, "version"), "version");
  symbols.entry = ParseHexWord(
      RequireString(RequireField(root, "entry"), "entry"), "entry");
  const int64_t rom_size = RequireNumber(RequireField(root, "rom_size"), "rom_size");
  if (rom_size <= 0) {
    throw SimError("rom_size must be positive");
  }
  symbols.rom_size = static_cast<uint32_t>(rom_size);

  const auto it_cartridge = obj.find("cartridge_version");
  if (it_cartridge != obj.end() && it_cartridge->second.type == JsonValue::Type::Number) {
    if (it_cartridge->second.number < 0) {
      throw SimError("cartridge_version must be non-negative");
    }
    symbols.cartridge_version =
        static_cast<uint32_t>(it_cartridge->second.number);
  }

  symbols.source_root = RequireString(
      RequireField(root, "source_root"), "source_root");
  symbols.source_files = ParseStringArray(
      RequireField(root, "source_files"), "source_files");

  const auto& symbols_obj = RequireObject(RequireField(root, "symbols")).object;
  for (const auto& [name, value] : symbols_obj) {
    symbols.symbols.emplace(name, ParseHexWord(
        RequireString(value, "symbols"), "symbols"));
  }

  const auto& pcs = RequireObject(RequireField(root, "pc_to_source")).object;
  for (const auto& [address, value] : pcs) {
    SourceLocation location;
    location.file = RequireString(RequireField(value, "file"), "pc_to_source.file");
    location.line = static_cast<int>(
        RequireNumber(RequireField(value, "line"), "pc_to_source.line"));
    location.column = static_cast<int>(
        RequireNumber(RequireField(value, "column"), "pc_to_source.column"));
    if (location.line < 1 || location.column < 1) {
      throw SimError("pc_to_source line/column must be positive");
    }
    location.text = RequireString(RequireField(value, "text"), "pc_to_source.text");
    const auto [_, inserted] = symbols.pc_to_source.emplace(
        ParseHexWord(address, "pc_to_source").value(), std::move(location));
    if (!inserted) {
      throw SimError("duplicate pc_to_source address");
    }
  }

  const auto& records = RequireArray(RequireField(root, "records"), "records").array;
  symbols.records.reserve(records.size());
  for (const auto& record_value : records) {
    DebugRecord entry;
    entry.address = ParseHexWord(
        RequireString(RequireField(record_value, "address"), "record.address"),
        "record.address");
    const int64_t rom_offset =
        RequireNumber(RequireField(record_value, "rom_offset"), "record.rom_offset");
    if (rom_offset < 0 || rom_offset >= static_cast<int64_t>(symbols.rom_size)) {
      throw SimError("record.rom_offset out of range");
    }
    entry.rom_offset = static_cast<uint32_t>(rom_offset);
    entry.location.file = RequireString(RequireField(record_value, "file"), "record.file");
    entry.location.line = static_cast<int>(
        RequireNumber(RequireField(record_value, "line"), "record.line"));
    entry.location.column = static_cast<int>(
        RequireNumber(RequireField(record_value, "column"), "record.column"));
    if (entry.location.line < 1 || entry.location.column < 1) {
      throw SimError("record line/column must be positive");
    }
    entry.location.text = RequireString(RequireField(record_value, "text"), "record.text");
    symbols.records.push_back(std::move(entry));
  }

  return symbols;
}

}  // namespace irata2::sim

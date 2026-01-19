#include "irata2/assembler/assembler.h"

#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "irata2/assembler/ast.h"
#include "irata2/assembler/error.h"
#include "irata2/assembler/lexer.h"
#include "irata2/assembler/parser.h"
#include "irata2/isa/isa.h"

namespace irata2::assembler {

namespace {
std::string ToLower(std::string_view text) {
  std::string out;
  out.reserve(text.size());
  for (char ch : text) {
    out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
  }
  return out;
}

struct Emittable {
  base::Word address;
  Span span;
  std::string text;
  enum class Kind { Instruction, Bytes } kind = Kind::Instruction;
  isa::AddressingMode addressing_mode = isa::AddressingMode::IMP;
  uint8_t opcode = 0;
  uint8_t operand_bytes = 0;
  std::vector<Operand> operands;
};

std::string FormatOperand(const Operand& operand) {
  if (operand.kind == Operand::Kind::Number) {
    return std::to_string(operand.number);
  }
  return operand.label;
}

std::string FormatInstructionText(const InstructionStmt& stmt) {
  std::ostringstream out;
  out << stmt.mnemonic;
  if (!stmt.operands.empty()) {
    out << " ";
    for (size_t i = 0; i < stmt.operands.size(); ++i) {
      out << FormatOperand(stmt.operands[i]);
      if (i + 1 < stmt.operands.size()) {
        out << ", ";
      }
    }
  }
  return out.str();
}

std::string FormatDirectiveText(const DirectiveStmt& stmt) {
  std::ostringstream out;
  if (stmt.type == DirectiveStmt::Type::Org) {
    out << ".org";
  } else {
    out << ".byte";
  }
  if (!stmt.operands.empty()) {
    out << " ";
    for (size_t i = 0; i < stmt.operands.size(); ++i) {
      out << FormatOperand(stmt.operands[i]);
      if (i + 1 < stmt.operands.size()) {
        out << ", ";
      }
    }
  }
  return out.str();
}

const isa::InstructionInfo* FindInstructionByMnemonic(std::string_view mnemonic) {
  const std::string lowered = ToLower(mnemonic);
  const auto& instructions = isa::IsaInfo::GetInstructions();
  for (const auto& inst : instructions) {
    if (ToLower(inst.mnemonic) == lowered) {
      return &inst;
    }
  }
  return nullptr;
}

const isa::InstructionInfo* FindInstructionByMnemonicAndMode(std::string_view mnemonic,
                                                              isa::AddressingMode mode) {
  const std::string lowered = ToLower(mnemonic);
  const auto& instructions = isa::IsaInfo::GetInstructions();
  for (const auto& inst : instructions) {
    if (ToLower(inst.mnemonic) == lowered && inst.addressing_mode == mode) {
      return &inst;
    }
  }
  return nullptr;
}

void RequireNumberOperand(const Operand& operand, const std::string& message) {
  if (operand.kind != Operand::Kind::Number) {
    throw AssemblerError(operand.span, message);
  }
}

bool IsRelativeMnemonic(std::string_view mnemonic) {
  const std::string lowered = ToLower(mnemonic);
  constexpr std::array<std::string_view, 8> kRelativeMnemonics = {
      "beq", "bne", "bcs", "bcc", "bmi", "bpl", "bvs", "bvc"};
  for (std::string_view candidate : kRelativeMnemonics) {
    if (candidate == lowered) {
      return true;
    }
  }
  return false;
}

const isa::InstructionInfo* SelectInstruction(const InstructionStmt& stmt) {
  std::vector<isa::AddressingMode> candidates;
  if (stmt.operands.empty()) {
    candidates = {isa::AddressingMode::IMP};
  } else if (stmt.operands.size() == 1) {
    const Operand& operand = stmt.operands.front();
    if (IsRelativeMnemonic(stmt.mnemonic)) {
      if (operand.immediate) {
        throw AssemblerError(stmt.span, "relative branches do not use immediate operands");
      }
      candidates = {isa::AddressingMode::REL};
    } else if (operand.indirect) {
      if (operand.index_register == Operand::IndexRegister::X) {
        candidates = {isa::AddressingMode::IZX};
      } else if (operand.index_register == Operand::IndexRegister::Y) {
        candidates = {isa::AddressingMode::IZY};
      } else {
        candidates = {isa::AddressingMode::IND};
      }
    } else if (operand.immediate) {
      candidates = {isa::AddressingMode::IMM};
    } else if (operand.index_register == Operand::IndexRegister::X) {
      // Indexed by X: ZPX for zero page, ABX for absolute
      if (operand.kind == Operand::Kind::Number && operand.number <= 0xFFu) {
        candidates = {isa::AddressingMode::ZPX, isa::AddressingMode::ABX};
      } else {
        candidates = {isa::AddressingMode::ABX};
      }
    } else if (operand.index_register == Operand::IndexRegister::Y) {
      // Indexed by Y: ZPY for zero page, ABY for absolute
      if (operand.kind == Operand::Kind::Number && operand.number <= 0xFFu) {
        candidates = {isa::AddressingMode::ZPY, isa::AddressingMode::ABY};
      } else {
        candidates = {isa::AddressingMode::ABY};
      }
    } else if (operand.kind == Operand::Kind::Number) {
      if (operand.number <= 0xFFu) {
        candidates = {isa::AddressingMode::ZP, isa::AddressingMode::ABS};
      } else {
        candidates = {isa::AddressingMode::ABS};
      }
    } else {
      candidates = {isa::AddressingMode::ABS, isa::AddressingMode::ZP};
    }
  } else {
    throw AssemblerError(stmt.span, "unexpected operand count");
  }

  for (isa::AddressingMode mode : candidates) {
    if (const auto* info = FindInstructionByMnemonicAndMode(stmt.mnemonic, mode)) {
      return info;
    }
  }

  throw AssemblerError(stmt.span, "unsupported addressing mode");
}

struct FirstPassResult {
  std::vector<Emittable> items;
  std::unordered_map<std::string, base::Word> symbols;
  base::Word origin;
  base::Word max_address;
};

base::Word AddOffset(base::Word address, uint32_t offset, const Span& span) {
  uint32_t value = address.value();
  value += offset;
  if (value > 0xFFFFu) {
    throw AssemblerError(span, "address overflow");
  }
  return base::Word{static_cast<uint16_t>(value)};
}

FirstPassResult FirstPass(const Program& program, const AssemblerOptions& options) {
  FirstPassResult result;
  result.origin = options.origin;
  result.max_address = options.origin;
  base::Word cursor = options.origin;

  for (const auto& statement : program.statements) {
    if (const auto* label = std::get_if<LabelDecl>(&statement)) {
      if (result.symbols.count(label->name) != 0) {
        throw AssemblerError(label->span, "duplicate label");
      }
      result.symbols.emplace(label->name, cursor);
      continue;
    }

    if (const auto* directive = std::get_if<DirectiveStmt>(&statement)) {
      if (directive->type == DirectiveStmt::Type::Org) {
        if (directive->operands.size() != 1) {
          throw AssemblerError(directive->span, ".org requires one operand");
        }
        RequireNumberOperand(directive->operands[0], ".org requires numeric literal");
        uint32_t value = directive->operands[0].number;
        if (value > 0xFFFFu) {
          throw AssemblerError(directive->operands[0].span, "origin out of range");
        }
        if (value < options.origin.value()) {
          throw AssemblerError(directive->operands[0].span, "origin below entry point");
        }
        cursor = base::Word{static_cast<uint16_t>(value)};
        if (cursor.value() > result.max_address.value()) {
          result.max_address = cursor;
        }
        continue;
      }

      if (directive->type == DirectiveStmt::Type::Byte) {
        if (directive->operands.empty()) {
          throw AssemblerError(directive->span, ".byte requires at least one operand");
        }
        Emittable item;
        item.kind = Emittable::Kind::Bytes;
        item.address = cursor;
        item.span = directive->span;
        item.text = FormatDirectiveText(*directive);
        item.operands = directive->operands;
        result.items.push_back(item);

        cursor = AddOffset(cursor, static_cast<uint32_t>(directive->operands.size()), directive->span);
        if (cursor.value() > result.max_address.value()) {
          result.max_address = cursor;
        }
      }

      continue;
    }

    if (const auto* instruction = std::get_if<InstructionStmt>(&statement)) {
      const isa::InstructionInfo* info = SelectInstruction(*instruction);
      if (!info) {
        throw AssemblerError(instruction->span, "unknown instruction mnemonic");
      }

      auto mode_info = isa::IsaInfo::GetAddressingMode(info->addressing_mode);
      if (!mode_info) {
        throw AssemblerError(instruction->span, "unknown addressing mode");
      }

      Emittable item;
      item.kind = Emittable::Kind::Instruction;
      item.address = cursor;
      item.span = instruction->span;
      item.text = FormatInstructionText(*instruction);
      item.opcode = static_cast<uint8_t>(info->opcode);
      item.addressing_mode = info->addressing_mode;
      item.operand_bytes = mode_info->operand_bytes;
      item.operands = instruction->operands;
      result.items.push_back(item);

      cursor = AddOffset(cursor, static_cast<uint32_t>(1 + mode_info->operand_bytes),
                         instruction->span);
      if (cursor.value() > result.max_address.value()) {
        result.max_address = cursor;
      }
      continue;
    }
  }

  return result;
}

uint8_t ResolveByteOperand(const Operand& operand,
                           const std::unordered_map<std::string, base::Word>& symbols) {
  if (operand.kind == Operand::Kind::Number) {
    if (operand.number > 0xFFu) {
      throw AssemblerError(operand.span, "byte literal out of range");
    }
    return static_cast<uint8_t>(operand.number);
  }

  auto it = symbols.find(operand.label);
  if (it == symbols.end()) {
    throw AssemblerError(operand.span, "unknown label");
  }
  uint32_t value = it->second.value();
  if (value > 0xFFu) {
    throw AssemblerError(operand.span, "label out of byte range");
  }
  return static_cast<uint8_t>(value);
}

uint16_t ResolveWordOperand(const Operand& operand,
                            const std::unordered_map<std::string, base::Word>& symbols) {
  if (operand.kind == Operand::Kind::Number) {
    if (operand.number > 0xFFFFu) {
      throw AssemblerError(operand.span, "word literal out of range");
    }
    return static_cast<uint16_t>(operand.number);
  }

  auto it = symbols.find(operand.label);
  if (it == symbols.end()) {
    throw AssemblerError(operand.span, "unknown label");
  }
  return static_cast<uint16_t>(it->second.value());
}

uint8_t ResolveRelativeOperand(const Operand& operand,
                               const std::unordered_map<std::string, base::Word>& symbols,
                               base::Word instruction_address) {
  uint32_t target = 0;
  if (operand.kind == Operand::Kind::Number) {
    if (operand.number > 0xFFFFu) {
      throw AssemblerError(operand.span, "relative branch target out of range");
    }
    target = operand.number;
  } else {
    auto it = symbols.find(operand.label);
    if (it == symbols.end()) {
      throw AssemblerError(operand.span, "unknown label");
    }
    target = it->second.value();
  }

  int32_t base = static_cast<int32_t>(instruction_address.value()) + 2;
  int32_t offset = static_cast<int32_t>(target) - base;
  if (offset < -128 || offset > 127) {
    throw AssemblerError(operand.span, "relative branch out of range");
  }
  return static_cast<uint8_t>(static_cast<int8_t>(offset));
}

AssemblerResult Encode(const FirstPassResult& pass,
                       const AssemblerOptions& options,
                       const std::string& source_file) {
  uint32_t origin = options.origin.value();
  uint32_t max_address = pass.max_address.value();
  if (max_address < origin) {
    max_address = origin;
  }

  uint32_t program_size = (max_address - origin);
  if (program_size > options.rom_size) {
    throw AssemblerError(Span{}, "program exceeds cartridge size");
  }

  std::vector<uint8_t> rom(options.rom_size, 0xFF);
  std::filesystem::path source_path =
      source_file.empty() ? std::filesystem::path("unknown.asm")
                          : std::filesystem::path(source_file);
  std::filesystem::path root_path = source_path.parent_path();
  std::string source_root = root_path.empty() ? "." : root_path.string();
  std::vector<std::string> source_files;
  std::unordered_map<std::string, bool> seen_files;

  auto RelativeToRoot = [&](const std::string& file) -> std::string {
    std::filesystem::path path = file.empty() ? source_path : std::filesystem::path(file);
    if (root_path.empty()) {
      return path.string();
    }
    std::filesystem::path relative = path.lexically_relative(root_path);
    if (relative.empty()) {
      return path.filename().string();
    }
    return relative.string();
  };

  auto TrackSourceFile = [&](const std::string& file) -> std::string {
    std::string relative = RelativeToRoot(file);
    if (seen_files.insert({relative, true}).second) {
      source_files.push_back(relative);
    }
    return relative;
  };

  std::vector<DebugRecord> records;
  records.reserve(pass.items.size());

  auto AddRecord = [&](const Span& span, base::Word address, uint32_t offset,
                       const std::string& text) {
    DebugRecord record;
    record.address = address;
    record.rom_offset = offset;
    record.file = TrackSourceFile(span.file);
    record.line = span.line;
    record.column = span.column;
    record.text = text;
    records.push_back(std::move(record));
  };

  for (const auto& item : pass.items) {
    uint32_t address = item.address.value();
    if (address < origin) {
      throw AssemblerError(item.span, "address below entry point");
    }
    uint32_t offset = address - origin;
    if (offset >= rom.size()) {
      throw AssemblerError(item.span, "address exceeds cartridge size");
    }

    if (item.kind == Emittable::Kind::Instruction) {
      rom[offset] = item.opcode;
      AddRecord(item.span, item.address, offset, item.text);
      if (item.operand_bytes == 1) {
        if (item.operands.empty()) {
          throw AssemblerError(item.span, "missing operand");
        }
        uint8_t value = item.addressing_mode == isa::AddressingMode::REL
                            ? ResolveRelativeOperand(item.operands[0], pass.symbols,
                                                     item.address)
                            : ResolveByteOperand(item.operands[0], pass.symbols);
        if (offset + 1 >= rom.size()) {
          throw AssemblerError(item.span, "address exceeds cartridge size");
        }
        rom[offset + 1] = value;
        AddRecord(item.span, base::Word{static_cast<uint16_t>(address + 1)},
                  offset + 1, item.text);
      } else if (item.operand_bytes == 2) {
        if (item.operands.empty()) {
          throw AssemblerError(item.span, "missing operand");
        }
        uint16_t value = ResolveWordOperand(item.operands[0], pass.symbols);
        if (offset + 2 >= rom.size()) {
          throw AssemblerError(item.span, "address exceeds cartridge size");
        }
        rom[offset + 1] = static_cast<uint8_t>(value & 0xFFu);
        rom[offset + 2] = static_cast<uint8_t>((value >> 8) & 0xFFu);
        for (uint32_t i = 1; i <= 2; ++i) {
          AddRecord(item.span, base::Word{static_cast<uint16_t>(address + i)},
                    offset + i, item.text);
        }
      } else if (item.operand_bytes != 0) {
        throw AssemblerError(item.span, "unsupported operand width");
      }
      continue;
    }

    for (size_t i = 0; i < item.operands.size(); ++i) {
      uint32_t target = offset + static_cast<uint32_t>(i);
      if (target >= rom.size()) {
        throw AssemblerError(item.span, "byte write exceeds cartridge size");
      }
      rom[target] = ResolveByteOperand(item.operands[i], pass.symbols);
      AddRecord(item.span, base::Word{static_cast<uint16_t>(address + i)},
                target, item.text);
    }
  }

  AssemblerResult result;
  result.header.entry = options.origin;
  result.header.rom_size = static_cast<uint32_t>(rom.size());
  result.rom = std::move(rom);
  if (source_files.empty()) {
    TrackSourceFile(source_path.string());
  }
  result.debug_json = EncodeDebugJson(result.header,
                                      "v1",
                                      source_root,
                                      source_files,
                                      pass.symbols,
                                      records);
  return result;
}

std::string ReadFile(const std::string& path) {
  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("failed to open asm file: " + path);
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void WriteFile(const std::string& path, const std::string& content) {
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    throw std::runtime_error("failed to write file: " + path);
  }
  out.write(content.data(), static_cast<std::streamsize>(content.size()));
}

void WriteFile(const std::string& path, const std::vector<uint8_t>& content) {
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    throw std::runtime_error("failed to write file: " + path);
  }
  out.write(reinterpret_cast<const char*>(content.data()),
            static_cast<std::streamsize>(content.size()));
}
}  // namespace

AssemblerResult Assemble(std::string_view source,
                         const std::string& filename,
                         const AssemblerOptions& options) {
  Lexer lexer(source, filename);
  std::vector<Token> tokens = lexer.Lex();
  Parser parser(std::move(tokens));
  Program program = parser.Parse();

  FirstPassResult pass = FirstPass(program, options);
  return Encode(pass, options, filename);
}

AssemblerResult AssembleFile(const std::string& path,
                             const AssemblerOptions& options) {
  std::string source = ReadFile(path);
  return Assemble(source, path, options);
}

void WriteCartridge(const AssemblerResult& result, const std::string& path) {
  std::vector<uint8_t> header = EncodeHeader(result.header);
  std::vector<uint8_t> output;
  output.reserve(header.size() + result.rom.size());
  output.insert(output.end(), header.begin(), header.end());
  output.insert(output.end(), result.rom.begin(), result.rom.end());
  WriteFile(path, output);
}

void WriteDebugInfo(const AssemblerResult& result, const std::string& path) {
  WriteFile(path, result.debug_json);
}

}  // namespace irata2::assembler

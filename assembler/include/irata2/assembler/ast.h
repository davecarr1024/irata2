#ifndef IRATA2_ASSEMBLER_AST_H
#define IRATA2_ASSEMBLER_AST_H

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "irata2/assembler/span.h"

namespace irata2::assembler {

struct Operand {
  enum class Kind { Number, Label };
  enum class IndexRegister { None, X, Y };

  Kind kind = Kind::Number;
  uint32_t number = 0;
  std::string label;
  bool immediate = false;
  bool indirect = false;
  IndexRegister index_register = IndexRegister::None;
  Span span;
};

struct LabelDecl {
  std::string name;
  Span span;
};

struct EquDecl {
  std::string name;
  uint32_t value;
  Span span;
};

struct InstructionStmt {
  std::string mnemonic;
  std::vector<Operand> operands;
  Span span;
};

struct DirectiveStmt {
  enum class Type { Org, Byte, Include };

  Type type = Type::Org;
  std::vector<Operand> operands;
  std::string include_path;  // For Include directive
  Span span;
};

using Statement = std::variant<LabelDecl, EquDecl, InstructionStmt, DirectiveStmt>;

struct Program {
  std::vector<Statement> statements;
};

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_AST_H

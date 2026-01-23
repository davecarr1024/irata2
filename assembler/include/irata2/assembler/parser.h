#ifndef IRATA2_ASSEMBLER_PARSER_H
#define IRATA2_ASSEMBLER_PARSER_H

#include <vector>

#include "irata2/assembler/ast.h"
#include "irata2/assembler/token.h"

namespace irata2::assembler {

class Parser final {
 public:
  explicit Parser(std::vector<Token> tokens);

  Program Parse();

 private:
  const Token& Peek() const;
  const Token& PeekNext() const;
  bool AtEnd() const;
  const Token& Advance();
  bool Match(TokenKind kind);
  const Token& Consume(TokenKind kind, const std::string& message);

  void ConsumeNewlines();
  bool Check(TokenKind kind) const;

  Operand ParseOperand();
  InstructionStmt ParseInstruction();
  Statement ParseDirective();

  std::vector<Token> tokens_;
  size_t index_ = 0;
};

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_PARSER_H

#ifndef IRATA2_ASSEMBLER_LEXER_H
#define IRATA2_ASSEMBLER_LEXER_H

#include <string>
#include <string_view>
#include <vector>

#include "irata2/assembler/token.h"

namespace irata2::assembler {

class Lexer final {
 public:
  Lexer(std::string_view source, std::string filename);

  std::vector<Token> Lex();

 private:
  Token NextToken();

  bool AtEnd() const;
  char Peek() const;
  char PeekNext() const;
  char Advance();
  void SkipWhitespace();
  void SkipComment();

  Token MakeToken(TokenKind kind, int length) const;
  Token LexIdentifierOrDirective();
  Token LexNumber();

  std::string_view source_;
  std::string filename_;
  size_t index_ = 0;
  int line_ = 1;
  int column_ = 1;
};

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_LEXER_H

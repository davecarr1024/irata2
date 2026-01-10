#ifndef IRATA2_ASSEMBLER_TOKEN_H
#define IRATA2_ASSEMBLER_TOKEN_H

#include <cstdint>
#include <string>
#include <optional>

#include "irata2/assembler/span.h"

namespace irata2::assembler {

enum class TokenKind {
  Identifier,
  Number,
  Directive,
  Comma,
  Colon,
  Newline,
  EndOfFile,
};

struct Token {
  TokenKind kind = TokenKind::EndOfFile;
  std::string text;
  Span span;
  std::optional<uint32_t> number;
};

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_TOKEN_H

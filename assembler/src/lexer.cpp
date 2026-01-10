#include "irata2/assembler/lexer.h"

#include <cctype>

#include "irata2/assembler/error.h"

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

bool IsIdentifierStart(char ch) {
  return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool IsIdentifierChar(char ch) {
  return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

uint32_t ParseNumberLiteral(std::string_view text, const Span& span) {
  std::string_view value = text;
  if (!value.empty() && value.front() == '#') {
    value.remove_prefix(1);
  }

  int base = 10;
  if (!value.empty() && value.front() == '$') {
    base = 16;
    value.remove_prefix(1);
  } else if (!value.empty() && value.front() == '%') {
    base = 2;
    value.remove_prefix(1);
  } else if (value.size() > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) {
    base = 16;
    value.remove_prefix(2);
  } else if (value.size() > 2 && value[0] == '0' && (value[1] == 'b' || value[1] == 'B')) {
    base = 2;
    value.remove_prefix(2);
  }

  if (value.empty()) {
    throw AssemblerError(span, "invalid numeric literal");
  }

  uint32_t parsed = 0;
  for (char ch : value) {
    int digit = -1;
    if (ch >= '0' && ch <= '9') {
      digit = ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
      digit = 10 + (ch - 'a');
    } else if (ch >= 'A' && ch <= 'F') {
      digit = 10 + (ch - 'A');
    }

    if (digit < 0 || digit >= base) {
      throw AssemblerError(span, "invalid numeric literal");
    }

    parsed = parsed * static_cast<uint32_t>(base) + static_cast<uint32_t>(digit);
  }

  return parsed;
}
}  // namespace

Lexer::Lexer(std::string_view source, std::string filename)
    : source_(source), filename_(std::move(filename)) {}

std::vector<Token> Lexer::Lex() {
  std::vector<Token> tokens;
  while (true) {
    Token token = NextToken();
    tokens.push_back(token);
    if (token.kind == TokenKind::EndOfFile) {
      break;
    }
  }
  return tokens;
}

Token Lexer::NextToken() {
  SkipWhitespace();

  if (AtEnd()) {
    return MakeToken(TokenKind::EndOfFile, 0);
  }

  char ch = Peek();
  if (ch == '\n') {
    Advance();
    Token token = MakeToken(TokenKind::Newline, 1);
    token.text = "\n";
    return token;
  }

  if (ch == ';') {
    SkipComment();
    return NextToken();
  }

  if (ch == ',') {
    Advance();
    return MakeToken(TokenKind::Comma, 1);
  }

  if (ch == ':') {
    Advance();
    return MakeToken(TokenKind::Colon, 1);
  }

  if (ch == '.') {
    return LexIdentifierOrDirective();
  }

  if (IsIdentifierStart(ch)) {
    return LexIdentifierOrDirective();
  }

  if (ch == '#' || ch == '$' || ch == '%' || std::isdigit(static_cast<unsigned char>(ch))) {
    return LexNumber();
  }

  Span span{filename_, line_, column_, 1};
  throw AssemblerError(span, "unexpected character in input");
}

bool Lexer::AtEnd() const {
  return index_ >= source_.size();
}

char Lexer::Peek() const {
  if (AtEnd()) {
    return '\0';
  }
  return source_[index_];
}

char Lexer::PeekNext() const {
  if (index_ + 1 >= source_.size()) {
    return '\0';
  }
  return source_[index_ + 1];
}

char Lexer::Advance() {
  char ch = source_[index_++];
  if (ch == '\n') {
    line_++;
    column_ = 1;
  } else {
    column_++;
  }
  return ch;
}

void Lexer::SkipWhitespace() {
  while (!AtEnd()) {
    char ch = Peek();
    if (ch == ' ' || ch == '\t' || ch == '\r') {
      Advance();
      continue;
    }
    break;
  }
}

void Lexer::SkipComment() {
  while (!AtEnd() && Peek() != '\n') {
    Advance();
  }
}

Token Lexer::MakeToken(TokenKind kind, int length) const {
  return Token{kind, std::string(), Span{filename_, line_, column_, length}, std::nullopt};
}

Token Lexer::LexIdentifierOrDirective() {
  int start_column = column_;
  size_t start = index_;
  bool is_directive = false;

  if (Peek() == '.') {
    is_directive = true;
    Advance();
    start = index_;
  }

  if (!IsIdentifierStart(Peek())) {
    Span span{filename_, line_, start_column, 1};
    throw AssemblerError(span, "expected identifier");
  }

  while (!AtEnd() && IsIdentifierChar(Peek())) {
    Advance();
  }

  size_t end = index_;
  std::string text = ToLower(source_.substr(start, end - start));
  int length = static_cast<int>(end - (is_directive ? start - 1 : start));
  Token token;
  token.kind = is_directive ? TokenKind::Directive : TokenKind::Identifier;
  token.text = std::move(text);
  token.span = Span{filename_, line_, start_column, length};
  return token;
}

Token Lexer::LexNumber() {
  int start_column = column_;
  size_t start = index_;

  if (Peek() == '#') {
    Advance();
  }

  if (Peek() == '$' || Peek() == '%') {
    Advance();
  } else if (Peek() == '0' && (PeekNext() == 'x' || PeekNext() == 'X' || PeekNext() == 'b' || PeekNext() == 'B')) {
    Advance();
    Advance();
  }

  while (!AtEnd()) {
    char ch = Peek();
    if (std::isalnum(static_cast<unsigned char>(ch))) {
      Advance();
    } else {
      break;
    }
  }

  size_t end = index_;
  std::string text = std::string(source_.substr(start, end - start));
  Span span{filename_, line_, start_column, static_cast<int>(end - start)};
  uint32_t value = ParseNumberLiteral(text, span);

  Token token;
  token.kind = TokenKind::Number;
  token.text = std::move(text);
  token.span = span;
  token.number = value;
  return token;
}

}  // namespace irata2::assembler

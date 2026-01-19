#include "irata2/assembler/parser.h"

#include "irata2/assembler/error.h"

namespace irata2::assembler {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

Program Parser::Parse() {
  Program program;

  while (!AtEnd()) {
    ConsumeNewlines();
    if (AtEnd()) {
      break;
    }

    bool parsed_label = false;
    while (Check(TokenKind::Identifier) && PeekNext().kind == TokenKind::Colon) {
      Token name = Advance();
      Token colon = Advance();
      (void)colon;
      program.statements.push_back(LabelDecl{name.text, name.span});
      parsed_label = true;
      if (Check(TokenKind::Newline)) {
        break;
      }
    }

    if (Check(TokenKind::Newline)) {
      Advance();
      continue;
    }

    if (Check(TokenKind::Directive)) {
      program.statements.push_back(ParseDirective());
    } else if (Check(TokenKind::Identifier)) {
      program.statements.push_back(ParseInstruction());
    } else if (parsed_label) {
      ConsumeNewlines();
      continue;
    } else {
      throw AssemblerError(Peek().span, "expected label, directive, or instruction");
    }

    if (Check(TokenKind::Newline)) {
      Advance();
    } else if (!AtEnd()) {
      throw AssemblerError(Peek().span, "expected end of line");
    }
  }

  return program;
}

const Token& Parser::Peek() const {
  return tokens_.at(index_);
}

const Token& Parser::PeekNext() const {
  if (index_ + 1 >= tokens_.size()) {
    return tokens_.back();
  }
  return tokens_.at(index_ + 1);
}

bool Parser::AtEnd() const {
  return Peek().kind == TokenKind::EndOfFile;
}

const Token& Parser::Advance() {
  if (!AtEnd()) {
    index_++;
  }
  return tokens_.at(index_ - 1);
}

bool Parser::Match(TokenKind kind) {
  if (Check(kind)) {
    Advance();
    return true;
  }
  return false;
}

const Token& Parser::Consume(TokenKind kind, const std::string& message) {
  if (Check(kind)) {
    return Advance();
  }
  throw AssemblerError(Peek().span, message);
}

void Parser::ConsumeNewlines() {
  while (Match(TokenKind::Newline)) {
  }
}

bool Parser::Check(TokenKind kind) const {
  if (AtEnd()) {
    return kind == TokenKind::EndOfFile;
  }
  return Peek().kind == kind;
}

Operand Parser::ParseOperand() {
  if (Match(TokenKind::LeftParen)) {
    Operand operand = ParseOperand();
    if (operand.immediate) {
      throw AssemblerError(operand.span, "indirect operands cannot be immediate");
    }
    operand.indirect = true;
    if (Match(TokenKind::Comma)) {
      Token reg = Consume(TokenKind::Identifier, "expected index register after comma");
      if (reg.text == "X" || reg.text == "x") {
        operand.index_register = Operand::IndexRegister::X;
      } else {
        throw AssemblerError(reg.span, "expected X index register inside parentheses");
      }
    }
    Consume(TokenKind::RightParen, "expected ')' after indirect operand");
    return operand;
  }

  if (Check(TokenKind::Number)) {
    Token token = Advance();
    Operand operand;
    operand.kind = Operand::Kind::Number;
    operand.number = token.number.value_or(0);
    operand.immediate = !token.text.empty() && token.text.front() == '#';
    operand.span = token.span;
    return operand;
  }

  if (Check(TokenKind::Identifier)) {
    Token token = Advance();
    Operand operand;
    operand.kind = Operand::Kind::Label;
    operand.label = token.text;
    operand.span = token.span;
    return operand;
  }

  throw AssemblerError(Peek().span, "expected operand");
}

InstructionStmt Parser::ParseInstruction() {
  Token mnemonic = Consume(TokenKind::Identifier, "expected instruction mnemonic");
  InstructionStmt stmt;
  stmt.mnemonic = mnemonic.text;
  stmt.span = mnemonic.span;

  if (Check(TokenKind::Newline) || Check(TokenKind::EndOfFile)) {
    return stmt;
  }

  while (true) {
    stmt.operands.push_back(ParseOperand());
    if (Match(TokenKind::Comma)) {
      // Check if next token is an index register (X or Y)
      if (Check(TokenKind::Identifier)) {
        Token next = Peek();
        if (next.text == "X" || next.text == "x") {
          Advance();  // Consume the X
          stmt.operands.back().index_register = Operand::IndexRegister::X;
          break;
        } else if (next.text == "Y" || next.text == "y") {
          Advance();  // Consume the Y
          stmt.operands.back().index_register = Operand::IndexRegister::Y;
          break;
        }
      }
      continue;
    }
    break;
  }

  return stmt;
}

DirectiveStmt Parser::ParseDirective() {
  Token directive = Consume(TokenKind::Directive, "expected directive");
  DirectiveStmt stmt;
  stmt.span = directive.span;

  if (directive.text == "org") {
    stmt.type = DirectiveStmt::Type::Org;
  } else if (directive.text == "byte") {
    stmt.type = DirectiveStmt::Type::Byte;
  } else {
    throw AssemblerError(directive.span, "unknown directive");
  }

  if (Check(TokenKind::Newline) || Check(TokenKind::EndOfFile)) {
    return stmt;
  }

  while (true) {
    stmt.operands.push_back(ParseOperand());
    if (Match(TokenKind::Comma)) {
      continue;
    }
    break;
  }

  return stmt;
}

}  // namespace irata2::assembler

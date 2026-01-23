#include "irata2/assembler/include_processor.h"

#include <fstream>
#include <sstream>

#include "irata2/assembler/error.h"
#include "irata2/assembler/lexer.h"
#include "irata2/assembler/parser.h"

namespace irata2::assembler {

namespace {
std::string ReadFile(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + path.string());
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}
}  // namespace

IncludeProcessor::IncludeProcessor(std::filesystem::path base_dir)
    : base_dir_(std::move(base_dir)) {}

Program IncludeProcessor::Process(const Program& program,
                                   const std::filesystem::path& base_dir) {
  IncludeProcessor processor(base_dir);
  return processor.ProcessProgram(program, base_dir);
}

Program IncludeProcessor::ProcessProgram(
    const Program& program,
    const std::filesystem::path& current_dir) {
  Program result;

  for (const Statement& stmt : program.statements) {
    if (const auto* directive = std::get_if<DirectiveStmt>(&stmt)) {
      if (directive->type == DirectiveStmt::Type::Include) {
        // Resolve include path relative to current file's directory
        std::filesystem::path include_path = current_dir / directive->include_path;

        // Normalize path to detect circular includes
        include_path = std::filesystem::canonical(include_path);

        // Check for circular includes
        if (include_stack_.contains(include_path)) {
          throw AssemblerError(directive->span,
                               "circular include detected: " + include_path.string());
        }

        // Load and parse the included file
        include_stack_.insert(include_path);
        Program included = LoadAndParse(include_path);
        include_stack_.erase(include_path);

        // Recursively process includes in the included file
        std::filesystem::path included_dir = include_path.parent_path();
        Program processed = ProcessProgram(included, included_dir);

        // Merge statements from included file
        result.statements.insert(result.statements.end(),
                                 processed.statements.begin(),
                                 processed.statements.end());
      } else {
        // Copy non-include directive
        result.statements.push_back(stmt);
      }
    } else {
      // Copy label or instruction
      result.statements.push_back(stmt);
    }
  }

  return result;
}

Program IncludeProcessor::LoadAndParse(const std::filesystem::path& path) {
  std::string source = ReadFile(path);
  Lexer lexer(source, path.string());
  std::vector<Token> tokens = lexer.Lex();
  Parser parser(std::move(tokens));
  return parser.Parse();
}

}  // namespace irata2::assembler

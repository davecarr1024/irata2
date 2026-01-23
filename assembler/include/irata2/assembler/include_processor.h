#ifndef IRATA2_ASSEMBLER_INCLUDE_PROCESSOR_H
#define IRATA2_ASSEMBLER_INCLUDE_PROCESSOR_H

#include <filesystem>
#include <set>
#include <string>

#include "irata2/assembler/ast.h"

namespace irata2::assembler {

// Process include directives in a program, recursively loading and inlining
// included files. Detects circular includes and resolves paths relative to
// the including file's directory.
class IncludeProcessor final {
 public:
  // Process includes in the program. base_dir is the directory containing
  // the root source file (for resolving relative paths).
  static Program Process(const Program& program,
                         const std::filesystem::path& base_dir);

 private:
  explicit IncludeProcessor(std::filesystem::path base_dir);

  Program ProcessProgram(const Program& program,
                         const std::filesystem::path& current_dir);

  Program LoadAndParse(const std::filesystem::path& path);

  std::filesystem::path base_dir_;
  std::set<std::filesystem::path> include_stack_;
};

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_INCLUDE_PROCESSOR_H

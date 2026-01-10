#ifndef IRATA2_ASSEMBLER_ERROR_H
#define IRATA2_ASSEMBLER_ERROR_H

#include <stdexcept>
#include <string>

#include "irata2/assembler/span.h"

namespace irata2::assembler {

class AssemblerError final : public std::runtime_error {
 public:
  AssemblerError(const Span& span, const std::string& message)
      : std::runtime_error(message), span_(span) {}

  const Span& span() const { return span_; }

 private:
  Span span_;
};

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_ERROR_H

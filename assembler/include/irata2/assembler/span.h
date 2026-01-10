#ifndef IRATA2_ASSEMBLER_SPAN_H
#define IRATA2_ASSEMBLER_SPAN_H

#include <string>

namespace irata2::assembler {

struct Span {
  std::string file;
  int line = 1;
  int column = 1;
  int length = 0;
};

}  // namespace irata2::assembler

#endif  // IRATA2_ASSEMBLER_SPAN_H

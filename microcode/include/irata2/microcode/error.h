#ifndef IRATA2_MICROCODE_ERROR_H
#define IRATA2_MICROCODE_ERROR_H

#include <stdexcept>

namespace irata2::microcode {

class MicrocodeError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

}  // namespace irata2::microcode

#endif  // IRATA2_MICROCODE_ERROR_H

#ifndef IRATA2_MICROCODE_IR_STEP_H
#define IRATA2_MICROCODE_IR_STEP_H

#include "irata2/hdl/control_info.h"

#include <vector>

namespace irata2::microcode::ir {

struct Step {
  int stage = 0;
  std::vector<const hdl::ControlInfo*> controls;
};

}  // namespace irata2::microcode::ir

#endif  // IRATA2_MICROCODE_IR_STEP_H

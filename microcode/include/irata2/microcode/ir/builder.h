#ifndef IRATA2_MICROCODE_IR_BUILDER_H
#define IRATA2_MICROCODE_IR_BUILDER_H

#include "irata2/hdl/cpu.h"
#include "irata2/microcode/error.h"

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace irata2::microcode::ir {

class Builder {
 public:
  explicit Builder(const hdl::Cpu& cpu);

  const hdl::ControlBase* RequireControl(std::string_view path,
                                        std::string_view context) const;
  std::vector<const hdl::ControlBase*> RequireControls(
      std::initializer_list<std::string_view> paths,
      std::string_view context) const;

 private:
  const hdl::Cpu& cpu_;
};

}  // namespace irata2::microcode::ir

#endif  // IRATA2_MICROCODE_IR_BUILDER_H

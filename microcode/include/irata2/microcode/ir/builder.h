#ifndef IRATA2_MICROCODE_IR_BUILDER_H
#define IRATA2_MICROCODE_IR_BUILDER_H

#include "irata2/hdl/control_info.h"
#include "irata2/microcode/ir/cpu_path_resolver.h"

#include <initializer_list>
#include <string_view>
#include <vector>

namespace irata2::microcode::ir {

/// Helper for building microcode IR from path strings.
/// Uses CpuPathResolver to convert paths to ControlInfo pointers.
class Builder {
 public:
  explicit Builder(const CpuPathResolver& resolver);

  const hdl::ControlInfo* RequireControl(std::string_view path,
                                         std::string_view context) const;
  std::vector<const hdl::ControlInfo*> RequireControls(
      std::initializer_list<std::string_view> paths,
      std::string_view context) const;

 private:
  const CpuPathResolver& resolver_;
};

}  // namespace irata2::microcode::ir

#endif  // IRATA2_MICROCODE_IR_BUILDER_H

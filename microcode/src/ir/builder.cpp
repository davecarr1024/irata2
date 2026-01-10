#include "irata2/microcode/ir/builder.h"

namespace irata2::microcode::ir {

Builder::Builder(const CpuPathResolver& resolver) : resolver_(resolver) {}

const hdl::ControlInfo* Builder::RequireControl(
    std::string_view path, std::string_view context) const {
  return resolver_.RequireControl(path, context);
}

std::vector<const hdl::ControlInfo*> Builder::RequireControls(
    std::initializer_list<std::string_view> paths,
    std::string_view context) const {
  std::vector<const hdl::ControlInfo*> controls;
  controls.reserve(paths.size());
  for (const auto& path : paths) {
    controls.push_back(RequireControl(path, context));
  }
  return controls;
}  // LCOV_EXCL_LINE

}  // namespace irata2::microcode::ir

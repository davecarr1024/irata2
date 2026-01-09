#include "irata2/microcode/ir/builder.h"

#include <sstream>

namespace irata2::microcode::ir {

Builder::Builder(const hdl::Cpu& cpu) : cpu_(cpu) {}

const hdl::ControlBase* Builder::RequireControl(std::string_view path,
                                                std::string_view context) const {
  try {
    return cpu_.ResolveControl(path);
  } catch (const hdl::PathResolutionError& error) {
    std::ostringstream message;
    message << "microcode control lookup failed";
    if (!context.empty()) {
      message << " (" << context << ")";
    }
    message << ": " << error.what();
    throw MicrocodeError(message.str());
  }
}

std::vector<const hdl::ControlBase*> Builder::RequireControls(
    std::initializer_list<std::string_view> paths,
    std::string_view context) const {
  std::vector<const hdl::ControlBase*> controls;
  controls.reserve(paths.size());
  for (const auto& path : paths) {
    controls.push_back(RequireControl(path, context));
  }
  return controls;
}  // LCOV_EXCL_LINE

}  // namespace irata2::microcode::ir

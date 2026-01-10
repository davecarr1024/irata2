#include "irata2/microcode/ir/cpu_path_resolver.h"

#include "irata2/hdl/traits.h"

#include <algorithm>
#include <sstream>

namespace irata2::microcode::ir {

CpuPathResolver::CpuPathResolver(const hdl::Cpu& cpu) {
  IndexControls(cpu);
}

void CpuPathResolver::IndexControls(const hdl::Cpu& cpu) {
  cpu.visit([&](const auto& component) {
    using T = std::decay_t<decltype(component)>;
    if constexpr (hdl::is_control_v<T>) {
      const auto& info = component.control_info();
      controls_by_path_.emplace(std::string(info.path), &info);
      control_paths_.emplace_back(info.path);
    }
  });
  std::sort(control_paths_.begin(), control_paths_.end());
}

const hdl::ControlInfo* CpuPathResolver::FindControl(
    std::string_view path) const {
  auto it = controls_by_path_.find(std::string(path));
  if (it == controls_by_path_.end()) {
    return nullptr;
  }
  return it->second;
}

const hdl::ControlInfo* CpuPathResolver::RequireControl(
    std::string_view path, std::string_view context) const {
  if (path.empty()) {
    std::ostringstream message;
    message << "control path is empty";
    if (!context.empty()) {
      message << " (" << context << ")";
    }
    throw MicrocodeError(message.str());
  }

  const auto* info = FindControl(path);
  if (info == nullptr) {
    std::ostringstream message;
    message << "control path not found: " << path;
    if (!context.empty()) {
      message << " (" << context << ")";
    }
    throw MicrocodeError(message.str());
  }
  return info;
}

}  // namespace irata2::microcode::ir

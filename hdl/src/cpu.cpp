#include "irata2/hdl/cpu.h"

#include "irata2/hdl/traits.h"

#include <algorithm>
#include <sstream>

namespace irata2::hdl {

Cpu::Cpu()
    : Component<Cpu>("cpu", this),
      halt_control_("halt", *this),
      crash_control_("crash", *this),
      data_bus_("data_bus", *this),
      address_bus_("address_bus", *this),
      a_("a", *this, data_bus_),
      x_("x", *this, data_bus_),
      pc_("pc", *this, address_bus_),
      mar_("mar", *this, address_bus_),
      status_("status", *this, data_bus_),
      controller_("controller", *this, data_bus_) {}

void Cpu::IndexControls() const {
  if (controls_indexed_) {
    return;
  }

  controls_by_path_.clear();
  control_paths_.clear();

  visit([&](const auto& component) {
    using T = std::decay_t<decltype(component)>;
    if constexpr (is_control_v<T>) {
      controls_by_path_.emplace(component.path(), &component);
      control_paths_.push_back(component.path());
    }
  });

  std::sort(control_paths_.begin(), control_paths_.end());
  controls_indexed_ = true;
}

const ControlBase* Cpu::ResolveControl(std::string_view path) const {
  if (path.empty()) {
    throw PathResolutionError("control path is empty");
  }

  IndexControls();
  auto it = controls_by_path_.find(std::string(path));
  if (it == controls_by_path_.end()) {
    std::ostringstream message;
    message << "control path not found: " << path;
    throw PathResolutionError(message.str());
  }

  return it->second;
}

std::vector<std::string> Cpu::AllControlPaths() const {
  IndexControls();
  return control_paths_;
}

}  // namespace irata2::hdl

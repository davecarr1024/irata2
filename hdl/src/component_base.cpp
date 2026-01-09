#include "irata2/hdl/component_base.h"

#include <cassert>

namespace irata2::hdl {

namespace {
std::string BuildPath(const ComponentBase* parent, const std::string& name) {
  if (parent == nullptr) {
    return "";
  }
  const auto& parent_path = parent->path();
  if (parent_path.empty()) {
    return name;
  }
  return parent_path + "." + name;
}
}  // namespace

ComponentBase::ComponentBase(std::string name,
                             const ComponentBase* parent,
                             Cpu* cpu)
    : name_(std::move(name)),
      path_(BuildPath(parent, name_)),
      cpu_(cpu),
      parent_(parent) {
  assert(cpu_ != nullptr);
}

ComponentBase::ComponentBase(std::string name, Cpu* cpu)
    : ComponentBase(std::move(name), nullptr, cpu) {}

}  // namespace irata2::hdl

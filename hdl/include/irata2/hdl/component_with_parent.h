#ifndef IRATA2_HDL_COMPONENT_WITH_PARENT_H
#define IRATA2_HDL_COMPONENT_WITH_PARENT_H

#include "irata2/hdl/component.h"

#include <utility>

namespace irata2::hdl {

template <typename Derived>
class ComponentWithParent : public Component<Derived> {
 public:
  ComponentWithParent(std::string name, ComponentBase& parent)
      : Component<Derived>(std::move(name), &parent, &parent.cpu()) {}

  const ComponentBase& parent() const { return *this->parent_ptr(); }
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_COMPONENT_WITH_PARENT_H

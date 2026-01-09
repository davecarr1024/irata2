#ifndef IRATA2_HDL_COMPONENT_H
#define IRATA2_HDL_COMPONENT_H

#include "irata2/hdl/component_base.h"

#include <utility>

namespace irata2::hdl {

template <typename Derived>
class Component : public ComponentBase {
 public:
  Derived& self() { return static_cast<Derived&>(*this); }
  const Derived& self() const { return static_cast<const Derived&>(*this); }

  template <typename Visitor>
  void visit(Visitor&& visitor) const {
    self().visit_impl(std::forward<Visitor>(visitor));
  }

 protected:
  Component(std::string name, const ComponentBase* parent, Cpu* cpu)
      : ComponentBase(std::move(name), parent, cpu) {}

  Component(std::string name, Cpu* cpu)
      : ComponentBase(std::move(name), cpu) {}
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_COMPONENT_H

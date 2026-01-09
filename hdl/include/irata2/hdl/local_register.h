#ifndef IRATA2_HDL_LOCAL_REGISTER_H
#define IRATA2_HDL_LOCAL_REGISTER_H

#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/process_control.h"
#include "irata2/hdl/traits.h"

#include <utility>

namespace irata2::hdl {

template <typename Derived, typename ValueType>
class LocalRegister : public ComponentWithParent<Derived>, public RegisterTag {
 public:
  using value_type = ValueType;

  LocalRegister(std::string name, ComponentBase& parent)
      : ComponentWithParent<Derived>(std::move(name), parent),
        reset_control_("reset", *this) {}

  const ProcessControl<true>& reset() const { return reset_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(static_cast<const Derived&>(*this));
    reset_control_.visit(visitor);
  }

 private:
  const ProcessControl<true> reset_control_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_LOCAL_REGISTER_H

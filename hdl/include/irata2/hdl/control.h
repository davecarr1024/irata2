#ifndef IRATA2_HDL_CONTROL_H
#define IRATA2_HDL_CONTROL_H

#include "irata2/base/tick_phase.h"
#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/control_base.h"
#include "irata2/hdl/traits.h"

#include <utility>

namespace irata2::hdl {

template <typename Derived,
          typename ValueType,
          base::TickPhase Phase,
          bool AutoReset>
class Control : public ComponentWithParent<Derived>,
                public ControlBase,
                public ControlTag {
 public:
  using value_type = ValueType;
  static constexpr base::TickPhase kPhase = Phase;
  static constexpr bool kAutoReset = AutoReset;

  Control(std::string name, ComponentBase& parent)
      : ComponentWithParent<Derived>(std::move(name), parent) {}

  base::TickPhase phase() const override { return Phase; }
  bool auto_reset() const override { return AutoReset; }
  const std::string& name() const override {
    return ComponentWithParent<Derived>::name();
  }
  const std::string& path() const override {
    return ComponentWithParent<Derived>::path();
  }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(static_cast<const Derived&>(*this));
  }
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CONTROL_H

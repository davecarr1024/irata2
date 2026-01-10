#ifndef IRATA2_HDL_CONTROL_H
#define IRATA2_HDL_CONTROL_H

#include "irata2/base/tick_phase.h"
#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/control_info.h"
#include "irata2/hdl/traits.h"

#include <utility>

namespace irata2::hdl {

/// CRTP-based control template with compile-time properties.
/// No virtual functions - all properties encoded in template parameters
/// and accessible via the ControlInfo struct.
template <typename Derived,
          typename ValueType,
          base::TickPhase Phase,
          bool AutoReset>
class Control : public ComponentWithParent<Derived>, public ControlTag {
 public:
  using value_type = ValueType;
  static constexpr base::TickPhase kPhase = Phase;
  static constexpr bool kAutoReset = AutoReset;

  Control(std::string name, ComponentBase& parent)
      : ComponentWithParent<Derived>(std::move(name), parent),
        control_info_{Phase, AutoReset, ComponentWithParent<Derived>::path()} {}

  /// Returns the pre-computed control info (no virtuals).
  const ControlInfo& control_info() const { return control_info_; }

  /// Compile-time phase accessor.
  static constexpr base::TickPhase phase() { return Phase; }

  /// Compile-time auto-reset accessor.
  static constexpr bool auto_reset() { return AutoReset; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(static_cast<const Derived&>(*this));
  }

 private:
  const ControlInfo control_info_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CONTROL_H

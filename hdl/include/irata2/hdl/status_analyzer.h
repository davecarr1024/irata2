#ifndef IRATA2_HDL_STATUS_ANALYZER_H
#define IRATA2_HDL_STATUS_ANALYZER_H

#include "irata2/base/types.h"
#include "irata2/hdl/component_with_bus.h"
#include "irata2/hdl/process_control.h"

#include <utility>

namespace irata2::hdl {

class StatusAnalyzer final : public ComponentWithBus<StatusAnalyzer, base::Byte> {
 public:
  StatusAnalyzer(std::string name, ComponentBase& parent, const Bus<base::Byte>& bus)
      : ComponentWithBus<StatusAnalyzer, base::Byte>(std::move(name), parent, bus),
        reset_control_("reset", *this) {}

  const ProcessControl<true>& reset() const { return reset_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    ComponentWithBus<StatusAnalyzer, base::Byte>::visit_impl(visitor);
    reset_control_.visit(visitor);
  }

 private:
  const ProcessControl<true> reset_control_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_STATUS_ANALYZER_H

#ifndef IRATA2_HDL_STATUS_REGISTER_H
#define IRATA2_HDL_STATUS_REGISTER_H

#include "irata2/base/types.h"
#include "irata2/hdl/register.h"
#include "irata2/hdl/status_analyzer.h"
#include "irata2/hdl/status.h"

#include <utility>

namespace irata2::hdl {

class StatusRegister final : public Register<StatusRegister, base::Byte> {
 public:
  StatusRegister(std::string name, ComponentBase& parent, const Bus<base::Byte>& bus)
      : Register<StatusRegister, base::Byte>(std::move(name), parent, bus),
        negative_("negative", *this, 7),
        overflow_("overflow", *this, 6),
        unused_("unused", *this, 5),
        brk_("break", *this, 4),
        decimal_("decimal", *this, 3),
        interrupt_disable_("interrupt_disable", *this, 2),
        zero_("zero", *this, 1),
        carry_("carry", *this, 0),
        analyzer_("analyzer", *this, bus) {}

  const Status& negative() const { return negative_; }
  const Status& overflow() const { return overflow_; }
  const Status& unused() const { return unused_; }
  const Status& brk() const { return brk_; }
  const Status& decimal() const { return decimal_; }
  const Status& interrupt_disable() const { return interrupt_disable_; }
  const Status& zero() const { return zero_; }
  const Status& carry() const { return carry_; }
  const StatusAnalyzer& analyzer() const { return analyzer_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    Register<StatusRegister, base::Byte>::visit_impl(visitor);
    negative_.visit(visitor);
    overflow_.visit(visitor);
    unused_.visit(visitor);
    brk_.visit(visitor);
    decimal_.visit(visitor);
    interrupt_disable_.visit(visitor);
    zero_.visit(visitor);
    carry_.visit(visitor);
    analyzer_.visit(visitor);
  }

 private:
  const Status negative_;
  const Status overflow_;
  const Status unused_;
  const Status brk_;
  const Status decimal_;
  const Status interrupt_disable_;
  const Status zero_;
  const Status carry_;
  const StatusAnalyzer analyzer_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_STATUS_REGISTER_H

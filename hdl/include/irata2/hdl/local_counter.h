#ifndef IRATA2_HDL_LOCAL_COUNTER_H
#define IRATA2_HDL_LOCAL_COUNTER_H

#include "irata2/hdl/local_register.h"
#include "irata2/hdl/process_control.h"

#include <utility>

namespace irata2::hdl {

template <typename ValueType>
class LocalCounter : public LocalRegister<LocalCounter<ValueType>, ValueType> {
 public:
  LocalCounter(std::string name, ComponentBase& parent)
      : LocalRegister<LocalCounter<ValueType>, ValueType>(std::move(name), parent),
        increment_control_("increment", *this) {}

  const ProcessControl<true>& increment() const { return increment_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    LocalRegister<LocalCounter<ValueType>, ValueType>::visit_impl(visitor);
    increment_control_.visit(visitor);
  }

 private:
  const ProcessControl<true> increment_control_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_LOCAL_COUNTER_H

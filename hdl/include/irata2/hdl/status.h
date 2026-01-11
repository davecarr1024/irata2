#ifndef IRATA2_HDL_STATUS_H
#define IRATA2_HDL_STATUS_H

#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/process_control.h"
#include "irata2/hdl/traits.h"

#include <utility>

namespace irata2::hdl {

class Status final : public ComponentWithParent<Status>, public StatusTag {
 public:
  Status(std::string name, ComponentBase& parent, int bit_index)
      : ComponentWithParent<Status>(std::move(name), parent),
        bit_index_(bit_index),
        set_control_("set", *this),
        clear_control_("clear", *this) {}

  int bit_index() const { return bit_index_; }
  const ProcessControl<true>& set() const { return set_control_; }
  const ProcessControl<true>& clear() const { return clear_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(*this);
    set_control_.visit(visitor);
    clear_control_.visit(visitor);
  }

 private:
  const int bit_index_;
  const ProcessControl<true> set_control_;
  const ProcessControl<true> clear_control_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_STATUS_H

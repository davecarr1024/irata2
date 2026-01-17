#ifndef IRATA2_HDL_LATCHED_WORD_REGISTER_H
#define IRATA2_HDL_LATCHED_WORD_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/process_control.h"
#include "irata2/hdl/traits.h"

namespace irata2::hdl {

class LatchedWordRegister final
    : public ComponentWithParent<LatchedWordRegister>,
      public RegisterTag {
 public:
  LatchedWordRegister(std::string name, ComponentBase& parent)
      : ComponentWithParent<LatchedWordRegister>(std::move(name), parent),
        latch_("latch", *this) {}

  const ProcessControl<true>& latch() const { return latch_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(*this);
    latch_.visit(visitor);
  }

 private:
  const ProcessControl<true> latch_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_LATCHED_WORD_REGISTER_H

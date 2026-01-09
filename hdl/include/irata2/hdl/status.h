#ifndef IRATA2_HDL_STATUS_H
#define IRATA2_HDL_STATUS_H

#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/traits.h"

#include <utility>

namespace irata2::hdl {

class Status final : public ComponentWithParent<Status>, public StatusTag {
 public:
  Status(std::string name, ComponentBase& parent, int bit_index)
      : ComponentWithParent<Status>(std::move(name), parent),
        bit_index_(bit_index) {}

  int bit_index() const { return bit_index_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(*this);
  }

 private:
  const int bit_index_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_STATUS_H

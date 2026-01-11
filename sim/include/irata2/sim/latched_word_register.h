#ifndef IRATA2_SIM_LATCHED_WORD_REGISTER_H
#define IRATA2_SIM_LATCHED_WORD_REGISTER_H

#include "irata2/base/types.h"
#include "irata2/sim/component.h"

#include <utility>

namespace irata2::sim {

class LatchedWordRegister final : public ComponentWithParent {
 public:
  LatchedWordRegister(std::string name, Component& parent)
      : ComponentWithParent(parent, std::move(name)) {}

  const base::Word& value() const { return value_; }
  void set_value(base::Word value) { value_ = value; }

 private:
  base::Word value_{};
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_LATCHED_WORD_REGISTER_H

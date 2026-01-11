#include "irata2/sim/status.h"

#include "irata2/sim/status_register.h"

namespace irata2::sim {

Status::Status(std::string name,
               Component& parent,
               StatusRegister& status,
               uint8_t bit_index)
    : ComponentWithParent(parent, std::move(name)),
      status_(status),
      bit_index_(bit_index),
      set_control_("set", *this),
      clear_control_("clear", *this) {}

bool Status::value() const {
  return ((status_.value().value() >> bit_index_) & 1U) != 0;
}

void Status::Set(bool value) {
  uint8_t current = status_.value().value();
  const uint8_t mask = static_cast<uint8_t>(1U << bit_index_);
  if (value) {
    current |= mask;
  } else {
    current &= static_cast<uint8_t>(~mask);
  }
  status_.set_value(base::Byte{current});
}

void Status::TickProcess() {
  const bool want_set = set_control_.asserted();
  const bool want_clear = clear_control_.asserted();
  if (!want_set && !want_clear) {
    return;
  }

  // If both are asserted, "set" wins.
  Set(want_set);
}

}  // namespace irata2::sim

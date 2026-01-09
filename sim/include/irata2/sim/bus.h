#ifndef IRATA2_SIM_BUS_H
#define IRATA2_SIM_BUS_H

#include <optional>
#include <string>
#include <string_view>

#include "irata2/base/tick_phase.h"
#include "irata2/sim/component.h"
#include "irata2/sim/error.h"

namespace irata2::sim {

template <typename ValueType>
class Bus : public ComponentWithParent {
 public:
  using value_type = ValueType;
  static constexpr size_t kWidth = sizeof(ValueType) * 8;

  Bus(std::string name, Component& parent)
      : ComponentWithParent(parent, std::move(name)) {}

  bool has_value() const { return value_.has_value(); }

  ValueType value() const {
    if (!value_) {
      throw SimError("bus has no value: " + path());
    }
    return *value_;
  }

  void Write(ValueType value, std::string_view writer_path) {
    if (current_phase() != base::TickPhase::Write) {
      throw SimError("bus write outside write phase: " + path());
    }
    if (!writer_path_.empty()) {
      throw SimError("bus already written: " + path());
    }
    value_ = value;
    writer_path_ = std::string(writer_path);
  }

  ValueType Read(std::string_view reader_path) const {
    if (current_phase() != base::TickPhase::Read) {
      throw SimError("bus read outside read phase: " + path());
    }
    if (!value_) {
      throw SimError("bus read before write: " + std::string(reader_path));
    }
    return *value_;
  }

  void TickClear() override {
    value_.reset();
    writer_path_.clear();
  }

 private:
  std::optional<ValueType> value_;
  std::string writer_path_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_BUS_H

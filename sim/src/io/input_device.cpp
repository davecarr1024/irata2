#include "irata2/sim/io/input_device.h"

namespace irata2::sim::io {

InputDevice::InputDevice(std::string name, Component& parent)
    : Module(std::move(name), parent) {}

base::Byte InputDevice::Read(base::Word address) const {
  const auto offset = address.value();

  switch (offset) {
    case input_reg::STATUS: {
      uint8_t status = 0;
      if (!empty()) {
        status |= input_status::NOT_EMPTY;
      }
      if (full()) {
        status |= input_status::FULL;
      }
      if (irq_pending()) {
        status |= input_status::IRQ_PENDING;
      }
      return base::Byte{status};
    }

    case input_reg::DATA:
      // Read and pop - but Read is const, so we need mutable
      // This is a design issue - MMIO reads often have side effects
      // For now, return peek value (actual pop happens in non-const context)
      return base::Byte{const_cast<InputDevice*>(this)->pop()};

    case input_reg::PEEK:
      return base::Byte{peek()};

    case input_reg::COUNT:
      return base::Byte{static_cast<uint8_t>(count_)};

    default:
      // Reserved registers return 0
      return base::Byte{0};
  }
}

void InputDevice::Write(base::Word address, base::Byte value) {
  const auto offset = address.value();

  switch (offset) {
    case input_reg::CONTROL:
      irq_enabled_ = (value.value() & input_control::IRQ_ENABLE) != 0;
      break;

    default:
      // Writes to read-only or reserved registers are ignored
      break;
  }
}

void InputDevice::inject_key(uint8_t key_code) {
  if (full()) {
    // Queue full - drop input (as per spec)
    return;
  }

  queue_[write_idx_] = key_code;
  write_idx_ = (write_idx_ + 1) % QUEUE_SIZE;
  ++count_;
}

uint8_t InputDevice::pop() {
  if (empty()) {
    return 0x00;  // Return 0 if empty (as per spec)
  }

  const uint8_t value = queue_[read_idx_];
  read_idx_ = (read_idx_ + 1) % QUEUE_SIZE;
  --count_;
  return value;
}

uint8_t InputDevice::peek() const {
  if (empty()) {
    return 0x00;  // Return 0 if empty (as per spec)
  }
  return queue_[read_idx_];
}

}  // namespace irata2::sim::io

#include "irata2/sim/memory/memory_address_register.h"

namespace irata2::sim::memory {

MemoryAddressRegister::BytePort::BytePort(std::string name,
                                          Component& parent,
                                          Bus<base::Byte>& data_bus,
                                          MemoryAddressRegister& mar,
                                          bool is_high)
    : ComponentWithParent(parent, std::move(name)),
      read_("read", *this, data_bus),
      write_("write", *this, data_bus),
      reset_("reset", *this),
      data_bus_(data_bus),
      mar_(mar),
      is_high_(is_high) {}

base::Byte MemoryAddressRegister::BytePort::CurrentValue() const {
  return is_high_ ? mar_.HighValue() : mar_.LowValue();
}

void MemoryAddressRegister::BytePort::SetValue(base::Byte value) {
  if (is_high_) {
    mar_.SetHigh(value);
  } else {
    mar_.SetLow(value);
  }
}

void MemoryAddressRegister::BytePort::TickWrite() {
  if (write_.asserted()) {
    data_bus_.Write(CurrentValue(), path());
  }
}

void MemoryAddressRegister::BytePort::TickRead() {
  if (read_.asserted()) {
    SetValue(data_bus_.Read(path()));
  }
}

void MemoryAddressRegister::BytePort::TickProcess() {
  if (reset_.asserted()) {
    SetValue(base::Byte{0});
  }
}

MemoryAddressRegister::MemoryAddressRegister(std::string name,
                                             Component& parent,
                                             Bus<base::Word>& address_bus,
                                             Bus<base::Byte>& data_bus)
    : Register<MemoryAddressRegister, base::Word>(std::move(name), parent, address_bus),
      data_bus_(data_bus),
      low_("low", *this, data_bus, *this, false),
      high_("high", *this, data_bus, *this, true) {}

base::Byte MemoryAddressRegister::LowValue() const {
  return value().low();
}

base::Byte MemoryAddressRegister::HighValue() const {
  return value().high();
}

void MemoryAddressRegister::SetLow(base::Byte byte) {
  const auto current = value();
  set_value(base::Word(current.high(), byte));
}

void MemoryAddressRegister::SetHigh(base::Byte byte) {
  const auto current = value();
  set_value(base::Word(byte, current.low()));
}

}  // namespace irata2::sim::memory

#ifndef IRATA2_SIM_ROM_STORAGE_H
#define IRATA2_SIM_ROM_STORAGE_H

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <vector>

#include "irata2/sim/component.h"
#include "irata2/sim/error.h"

namespace irata2::sim {

/// Generic ROM storage component with configurable address and data types.
///
/// RomStorage is a simple read-only memory component that can be configured
/// with different address and data widths. It participates in the component
/// tree but has no tick behavior.
///
/// Common instantiations:
/// - RomStorage<base::Word, base::Byte> (16-bit address, 8-bit data) - memory
/// - RomStorage<uint32_t, uint64_t> (32-bit address, 64-bit data) - microcode
template <typename AddressType, typename DataType>
class RomStorage final : public ComponentWithParent {
 public:
  /// Construct a ROM filled with a specific value.
  RomStorage(std::string name,
             Component& parent,
             size_t size,
             DataType fill)
      : ComponentWithParent(parent, std::move(name)), data_(size, fill) {}

  /// Construct a ROM from existing data.
  RomStorage(std::string name, Component& parent, std::vector<DataType> data)
      : ComponentWithParent(parent, std::move(name)),
        data_(std::move(data)) {}

  /// Get the size of the ROM in data elements.
  size_t size() const { return data_.size(); }

  /// Read a value from the ROM.
  ///
  /// \param address The address to read from
  /// \return The value at the address
  /// \throws SimError if address is out of bounds
  DataType Read(AddressType address) const {
    const size_t index = static_cast<size_t>(address);
    if (index >= data_.size()) {
      std::ostringstream message;
      message << "ROM read out of bounds at " << path() << ": index "
              << index << " (size " << data_.size() << ")";
      throw SimError(message.str());
    }
    return data_[index];
  }

  /// Write to ROM (no-op, ROM is read-only).
  ///
  /// This method exists for interface compatibility but does nothing.
  /// ROM contents are set at construction time only.
  void Write(AddressType address, DataType value) {
    // ROM is read-only - writes are ignored
    (void)address;
    (void)value;
  }

 private:
  std::vector<DataType> data_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_ROM_STORAGE_H

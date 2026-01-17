#ifndef IRATA2_SIM_MEMORY_MODULE_H
#define IRATA2_SIM_MEMORY_MODULE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "irata2/base/types.h"
#include "irata2/sim/component.h"
#include "irata2/sim/error.h"
#include "irata2/sim/rom_storage.h"

namespace irata2::sim::memory {

/// Type alias for memory ROM storage (size_t address, 8-bit data)
using MemoryRomStorage = RomStorage<size_t, base::Byte>;

/// Base class for memory modules (RAM/ROM).
///
/// Modules are components that store data. They extend ComponentWithParent
/// to participate in the component tree and tick propagation.
class Module : public ComponentWithParent {
 public:
  Module(std::string name, Component& parent)
      : ComponentWithParent(parent, std::move(name)) {}

  virtual ~Module() = default;

  virtual size_t size() const = 0;
  virtual base::Byte Read(base::Word address) const = 0;
  virtual void Write(base::Word address, base::Byte value) = 0;
};

/// RAM module that allows both read and write operations.
class Ram final : public Module {
 public:
  Ram(std::string name, Component& parent, size_t size, base::Byte fill);

  size_t size() const override { return data_.size(); }
  base::Byte Read(base::Word address) const override;
  void Write(base::Word address, base::Byte value) override;

 private:
  std::vector<base::Byte> data_;
};

/// ROM module that only allows read operations.
class Rom final : public Module {
 public:
  Rom(std::string name, Component& parent, size_t size, base::Byte fill);
  Rom(std::string name, Component& parent, std::vector<base::Byte> data);

  size_t size() const override { return storage_.size(); }
  base::Byte Read(base::Word address) const override;
  void Write(base::Word address, base::Byte value) override;

  const MemoryRomStorage& storage() const { return storage_; }

 private:
  MemoryRomStorage storage_;
};

}  // namespace irata2::sim::memory

#endif  // IRATA2_SIM_MEMORY_MODULE_H

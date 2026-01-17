#ifndef IRATA2_SIM_MEMORY_MEMORY_H
#define IRATA2_SIM_MEMORY_MEMORY_H

#include <functional>
#include <utility>
#include <vector>

#include "irata2/base/types.h"
#include "irata2/sim/component_with_bus.h"
#include "irata2/sim/memory/memory_address_register.h"
#include "irata2/sim/memory/region.h"

namespace irata2::sim::memory {

class Memory final : public ComponentWithBus<Memory, base::Byte> {
 public:
  using RegionFactory = std::function<std::unique_ptr<Region>(Memory&)>;

  Memory(std::string name,
         Component& parent,
         Bus<base::Byte>& data_bus,
         Bus<base::Word>& address_bus,
         std::vector<RegionFactory> region_factories);

  MemoryAddressRegister& mar() { return mar_; }
  const MemoryAddressRegister& mar() const { return mar_; }

  base::Byte ReadAt(base::Word address) const;
  void WriteAt(base::Word address, base::Byte value);

 protected:
  // Implement ComponentWithBus abstract interface
  base::Byte read_value() const override { return ReadAt(mar_.value()); }
  void write_value(base::Byte value) override { WriteAt(mar_.value(), value); }

 private:
  Region* FindRegion(base::Word address);
  const Region* FindRegion(base::Word address) const;

  MemoryAddressRegister mar_;
  std::vector<std::unique_ptr<Region>> regions_;
};

}  // namespace irata2::sim::memory

#endif  // IRATA2_SIM_MEMORY_MEMORY_H

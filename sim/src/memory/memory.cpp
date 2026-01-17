#include "irata2/sim/memory/memory.h"

#include <sstream>

namespace irata2::sim::memory {

Memory::Memory(std::string name,
               Component& parent,
               Bus<base::Byte>& data_bus,
               Bus<base::Word>& address_bus,
               std::vector<RegionFactory> region_factories)
    : ComponentWithBus<Memory, base::Byte>(std::move(name), parent, data_bus),
      mar_("mar", *this, address_bus, data_bus) {
  // Build regions using factory pattern
  regions_.reserve(region_factories.size());
  for (auto& factory : region_factories) {
    regions_.push_back(factory(*this));
  }

  // Validate no overlapping regions
  for (size_t i = 0; i < regions_.size(); ++i) {
    for (size_t j = i + 1; j < regions_.size(); ++j) {
      if (regions_[i]->Overlaps(*regions_[j])) {
        std::ostringstream message;
        message << "memory regions overlap: " << regions_[i]->path()
                << " vs " << regions_[j]->path();
        throw SimError(message.str());
      }
    }
  }
}

Region* Memory::FindRegion(base::Word address) {
  for (auto& region : regions_) {
    if (region->Contains(address)) {
      return region.get();
    }
  }
  return nullptr;
}

const Region* Memory::FindRegion(base::Word address) const {
  for (const auto& region : regions_) {
    if (region->Contains(address)) {
      return region.get();
    }
  }
  return nullptr;
}

base::Byte Memory::ReadAt(base::Word address) const {
  const auto* region = FindRegion(address);
  if (!region) {
    return base::Byte{0xFF};
  }
  return region->Read(address);
}

void Memory::WriteAt(base::Word address, base::Byte value) {
  auto* region = FindRegion(address);
  if (!region) {
    std::ostringstream message;
    message << "memory write to unmapped address " << address.value();
    throw SimError(message.str());
  }
  region->Write(address, value);
}

}  // namespace irata2::sim::memory

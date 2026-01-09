#include "irata2/sim/memory/region.h"

#include <sstream>

namespace irata2::sim::memory {

namespace {
bool IsPowerOfTwo(size_t value) {
  return value != 0 && (value & (value - 1)) == 0;
}
}  // namespace

Region::Region(std::string name,
               base::Word offset,
               std::shared_ptr<Module> module)
    : name_(std::move(name)), offset_(offset), module_(std::move(module)) {
  if (!module_) {
    throw SimError("memory region module is null");
  }
  const auto region_size = module_->size();
  if (!IsPowerOfTwo(region_size)) {
    std::ostringstream message;
    message << "memory region size is not power of two: " << region_size;
    throw SimError(message.str());
  }
  if (offset_.value() % region_size != 0) {
    std::ostringstream message;
    message << "memory region offset not aligned: " << offset_.value();
    throw SimError(message.str());
  }
}

bool Region::Contains(base::Word address) const {
  const uint32_t lower = offset_.value();
  const uint32_t upper = lower + static_cast<uint32_t>(size());
  const uint32_t value = address.value();
  return value >= lower && value < upper;
}

bool Region::Overlaps(const Region& other) const {
  const uint32_t lower = offset_.value();
  const uint32_t upper = lower + static_cast<uint32_t>(size());
  const uint32_t other_lower = other.offset_.value();
  const uint32_t other_upper =
      other_lower + static_cast<uint32_t>(other.size());
  return lower < other_upper && other_lower < upper;
}

base::Word Region::Translate(base::Word address) const {
  return base::Word(address.value() - offset_.value());
}

base::Byte Region::Read(base::Word address) const {
  if (!Contains(address)) {
    std::ostringstream message;
    message << "address out of region range: " << address.value();
    throw SimError(message.str());
  }
  return module_->Read(Translate(address));
}

void Region::Write(base::Word address, base::Byte value) {
  if (!Contains(address)) {
    std::ostringstream message;
    message << "address out of region range: " << address.value();
    throw SimError(message.str());
  }
  module_->Write(Translate(address), value);
}

}  // namespace irata2::sim::memory

#include "irata2/sim/memory/module.h"

#include <sstream>

namespace irata2::sim::memory {

namespace {
void ValidateSize(size_t size) {
  if (size == 0) {
    throw SimError("memory module size must be non-zero");
  }
}
}  // namespace

Ram::Ram(std::string name, Component& parent, size_t size, base::Byte fill)
    : Module(std::move(name), parent), data_(size, fill) {
  ValidateSize(size);
}

base::Byte Ram::Read(base::Word address) const {
  const auto index = address.value();
  if (index >= data_.size()) {
    std::ostringstream message;
    message << "RAM read out of range: " << index;
    throw SimError(message.str());
  }
  return data_[index];
}

void Ram::Write(base::Word address, base::Byte value) {
  const auto index = address.value();
  if (index >= data_.size()) {
    std::ostringstream message;
    message << "RAM write out of range: " << index;
    throw SimError(message.str());
  }
  data_[index] = value;
}

Rom::Rom(std::string name, Component& parent, size_t size, base::Byte fill)
    : Module(std::move(name), parent), data_(size, fill) {
  ValidateSize(size);
}

Rom::Rom(std::string name, Component& parent, std::vector<base::Byte> data)
    : Module(std::move(name), parent), data_(std::move(data)) {
  ValidateSize(data_.size());
}

base::Byte Rom::Read(base::Word address) const {
  const auto index = address.value();
  if (index >= data_.size()) {
    std::ostringstream message;
    message << "ROM read out of range: " << index;
    throw SimError(message.str());
  }
  return data_[index];
}

void Rom::Write(base::Word address, base::Byte value) {
  (void)value;
  std::ostringstream message;
  message << "ROM write forbidden at address " << address.value();
  throw SimError(message.str());
}

}  // namespace irata2::sim::memory

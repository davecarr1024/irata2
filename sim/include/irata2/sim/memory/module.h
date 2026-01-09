#ifndef IRATA2_SIM_MEMORY_MODULE_H
#define IRATA2_SIM_MEMORY_MODULE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "irata2/base/types.h"
#include "irata2/sim/error.h"

namespace irata2::sim::memory {

class Module {
 public:
  virtual ~Module() = default;

  virtual size_t size() const = 0;
  virtual base::Byte Read(base::Word address) const = 0;
  virtual void Write(base::Word address, base::Byte value) = 0;
};

class Ram final : public Module {
 public:
  Ram(size_t size, base::Byte fill);

  size_t size() const override { return data_.size(); }
  base::Byte Read(base::Word address) const override;
  void Write(base::Word address, base::Byte value) override;

 private:
  std::vector<base::Byte> data_;
};

class Rom final : public Module {
 public:
  Rom(size_t size, base::Byte fill);

  size_t size() const override { return data_.size(); }
  base::Byte Read(base::Word address) const override;
  void Write(base::Word address, base::Byte value) override;

 private:
  std::vector<base::Byte> data_;
};

std::shared_ptr<Module> MakeRam(size_t size, base::Byte fill = base::Byte{0x00});
std::shared_ptr<Module> MakeRom(size_t size, base::Byte fill = base::Byte{0xFF});

}  // namespace irata2::sim::memory

#endif  // IRATA2_SIM_MEMORY_MODULE_H

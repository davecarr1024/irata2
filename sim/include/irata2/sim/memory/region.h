#ifndef IRATA2_SIM_MEMORY_REGION_H
#define IRATA2_SIM_MEMORY_REGION_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "irata2/base/types.h"
#include "irata2/sim/error.h"
#include "irata2/sim/memory/module.h"

namespace irata2::sim::memory {

class Region {
 public:
  Region(std::string name, base::Word offset, std::shared_ptr<Module> module);

  const std::string& name() const { return name_; }
  base::Word offset() const { return offset_; }
  size_t size() const { return module_->size(); }

  bool Contains(base::Word address) const;
  bool Overlaps(const Region& other) const;

  base::Byte Read(base::Word address) const;
  void Write(base::Word address, base::Byte value);

 private:
  base::Word Translate(base::Word address) const;

  std::string name_;
  base::Word offset_;
  std::shared_ptr<Module> module_;
};

}  // namespace irata2::sim::memory

#endif  // IRATA2_SIM_MEMORY_REGION_H

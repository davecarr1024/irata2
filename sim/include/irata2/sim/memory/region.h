#ifndef IRATA2_SIM_MEMORY_REGION_H
#define IRATA2_SIM_MEMORY_REGION_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/component.h"
#include "irata2/sim/error.h"
#include "irata2/sim/memory/module.h"

namespace irata2::sim::memory {

/// Memory region that maps a module at a specific offset.
///
/// Regions are components that own a module and provide address translation.
/// They extend ComponentWithParent to participate in the component tree.
class Region : public ComponentWithParent {
 public:
  using ModuleFactory = std::function<std::unique_ptr<Module>(Region&)>;

  Region(std::string name,
         Component& parent,
         base::Word offset,
         ModuleFactory module_factory);

  base::Word offset() const { return offset_; }
  size_t size() const { return module_->size(); }

  bool Contains(base::Word address) const;
  bool Overlaps(const Region& other) const;

  base::Byte Read(base::Word address) const;
  void Write(base::Word address, base::Byte value);

 private:
  base::Word Translate(base::Word address) const;

  base::Word offset_;
  std::unique_ptr<Module> module_;
};

}  // namespace irata2::sim::memory

#endif  // IRATA2_SIM_MEMORY_REGION_H

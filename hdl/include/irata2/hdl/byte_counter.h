#ifndef IRATA2_HDL_BYTE_COUNTER_H
#define IRATA2_HDL_BYTE_COUNTER_H

#include "irata2/base/types.h"
#include "irata2/hdl/local_counter.h"

#include <utility>

namespace irata2::hdl {

class ByteCounter final : public LocalCounter<base::Byte> {
 public:
  ByteCounter(std::string name, ComponentBase& parent)
      : LocalCounter<base::Byte>(std::move(name), parent) {}
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_BYTE_COUNTER_H

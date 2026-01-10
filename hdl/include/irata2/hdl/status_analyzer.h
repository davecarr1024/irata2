#ifndef IRATA2_HDL_STATUS_ANALYZER_H
#define IRATA2_HDL_STATUS_ANALYZER_H

#include "irata2/base/types.h"
#include "irata2/hdl/register.h"

#include <utility>

namespace irata2::hdl {

class StatusAnalyzer final : public Register<StatusAnalyzer, base::Byte> {
 public:
  StatusAnalyzer(std::string name, ComponentBase& parent, const Bus<base::Byte>& bus)
      : Register<StatusAnalyzer, base::Byte>(std::move(name), parent, bus) {}
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_STATUS_ANALYZER_H

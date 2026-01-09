#ifndef IRATA2_HDL_CONTROL_BASE_H
#define IRATA2_HDL_CONTROL_BASE_H

#include "irata2/base/tick_phase.h"

#include <string>

namespace irata2::hdl {

class ControlBase {
 public:
  virtual ~ControlBase() = default;

  virtual base::TickPhase phase() const = 0;
  virtual bool auto_reset() const = 0;
  virtual const std::string& name() const = 0;
  virtual const std::string& path() const = 0;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CONTROL_BASE_H

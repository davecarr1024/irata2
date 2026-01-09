#ifndef IRATA2_SIM_ERROR_H
#define IRATA2_SIM_ERROR_H

#include <stdexcept>

namespace irata2::sim {

class SimError final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_ERROR_H

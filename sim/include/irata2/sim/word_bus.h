#ifndef IRATA2_SIM_WORD_BUS_H
#define IRATA2_SIM_WORD_BUS_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/bus.h"

namespace irata2::sim {

class WordBus final : public Bus<base::Word> {
 public:
  WordBus(std::string name, Component& parent)
      : Bus<base::Word>(std::move(name), parent) {}
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_WORD_BUS_H

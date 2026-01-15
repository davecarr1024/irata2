#ifndef IRATA2_SIM_WORD_REGISTER_H
#define IRATA2_SIM_WORD_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/register_with_bus.h"

namespace irata2::sim {

class WordRegister : public RegisterWithBus<WordRegister, base::Word> {
 public:
  WordRegister(std::string name, Component& parent, Bus<base::Word>& bus)
      : RegisterWithBus<WordRegister, base::Word>(std::move(name), parent, bus) {}
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_WORD_REGISTER_H

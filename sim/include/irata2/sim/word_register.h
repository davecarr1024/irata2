#ifndef IRATA2_SIM_WORD_REGISTER_H
#define IRATA2_SIM_WORD_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/register.h"

namespace irata2::sim {

class WordRegister : public Register<WordRegister, base::Word> {
 public:
  WordRegister(std::string name, Component& parent, Bus<base::Word>& bus)
      : Register<WordRegister, base::Word>(std::move(name), parent, bus) {}
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_WORD_REGISTER_H

#ifndef IRATA2_HDL_WORD_REGISTER_H
#define IRATA2_HDL_WORD_REGISTER_H

#include "irata2/base/types.h"
#include "irata2/hdl/register.h"

#include <utility>

namespace irata2::hdl {

class WordRegister final : public Register<WordRegister, base::Word> {
 public:
  WordRegister(std::string name, ComponentBase& parent, const Bus<base::Word>& bus)
      : Register<WordRegister, base::Word>(std::move(name), parent, bus) {}
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_WORD_REGISTER_H

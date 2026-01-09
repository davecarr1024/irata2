#ifndef IRATA2_HDL_CPU_H
#define IRATA2_HDL_CPU_H

#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/component.h"
#include "irata2/hdl/counter.h"
#include "irata2/hdl/word_bus.h"
#include "irata2/hdl/word_register.h"

#include <utility>

namespace irata2::hdl {

class Cpu final : public Component<Cpu> {
 public:
  Cpu();

  const ByteBus& data_bus() const { return data_bus_; }
  const WordBus& address_bus() const { return address_bus_; }

  const ByteRegister& a() const { return a_; }
  const ByteRegister& x() const { return x_; }
  const Counter<base::Word>& pc() const { return pc_; }
  const WordRegister& mar() const { return mar_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(*this);
    data_bus_.visit(visitor);
    address_bus_.visit(visitor);
    a_.visit(visitor);
    x_.visit(visitor);
    pc_.visit(visitor);
    mar_.visit(visitor);
  }

 private:
  const ByteBus data_bus_;
  const WordBus address_bus_;
  const ByteRegister a_;
  const ByteRegister x_;
  const Counter<base::Word> pc_;
  const WordRegister mar_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CPU_H

#ifndef IRATA2_MICROCODE_COMPILER_BUS_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_BUS_VALIDATOR_H

#include "irata2/microcode/compiler/pass.h"

#include <cstddef>
#include <unordered_map>

namespace irata2::hdl {
class Cpu;
struct ControlInfo;
}  // namespace irata2::hdl

namespace irata2::microcode::compiler {

/**
 * @brief Validates bus usage patterns in microcode.
 *
 * Enforces bus discipline for each step:
 * - At most one writer per bus per step
 * - Readers on a bus require a corresponding writer in the same step
 * - Writers on a bus should have at least one reader (warning, not error)
 */
class BusValidator final : public Pass {
 public:
  explicit BusValidator(const hdl::Cpu& cpu);

  void Run(ir::InstructionSet& instruction_set) const override;
  size_t bus_control_count() const { return control_bus_map_.size(); }

 private:
  enum class BusType {
    kData,
    kAddress,
    kNone
  };

  enum class BusOperation {
    kRead,
    kWrite,
    kNone
  };

  struct BusInfo {
    BusType bus_type;
    BusOperation operation;
  };

  void BuildBusMap(const hdl::Cpu& cpu);
  void ValidateStep(const ir::Step& step, int opcode, int step_index) const;

  std::unordered_map<const hdl::ControlInfo*, BusInfo> control_bus_map_;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_BUS_VALIDATOR_H

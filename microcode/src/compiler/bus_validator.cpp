#include "irata2/microcode/compiler/bus_validator.h"

#include "irata2/base/types.h"
#include "irata2/hdl/cpu.h"
#include "irata2/hdl/traits.h"
#include "irata2/microcode/error.h"

#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace irata2::microcode::compiler {

BusValidator::BusValidator(const hdl::Cpu& cpu) {
  BuildBusMap(cpu);
}

void BusValidator::BuildBusMap(const hdl::Cpu& cpu) {
  cpu.visit([&](const auto& component) {
    using T = std::decay_t<decltype(component)>;
    if constexpr (hdl::is_control_v<T>) {
      if constexpr (requires { component.bus(); } &&
                    requires { typename T::value_type; }) {
        using ValueType = typename T::value_type;
        BusType bus_type = BusType::kNone;
        if constexpr (std::is_same_v<ValueType, base::Byte>) {
          bus_type = BusType::kData;
        } else if constexpr (std::is_same_v<ValueType, base::Word>) {
          bus_type = BusType::kAddress;
        }
        if (bus_type == BusType::kNone) {
          return;
        }

        BusOperation operation = BusOperation::kNone;
        switch (component.control_info().phase) {
          case base::TickPhase::Read:
            operation = BusOperation::kRead;
            break;
          case base::TickPhase::Write:
            operation = BusOperation::kWrite;
            break;
          default:
            return;
        }
        control_bus_map_.emplace(&component.control_info(),
                                 BusInfo{bus_type, operation});
      }
    }
  });
}

void BusValidator::ValidateStep(const ir::Step& step,
                                int opcode,
                                int step_index) const {
  // Track writers and readers for each bus
  std::map<BusType, std::vector<std::string>> writers;
  std::map<BusType, std::vector<std::string>> readers;

  for (const auto* control : step.controls) {
    auto it = control_bus_map_.find(control);
    if (it == control_bus_map_.end()) {
      continue;
    }

    const auto bus_info = it->second;
    const std::string path(control->path);
    if (bus_info.operation == BusOperation::kWrite) {
      writers[bus_info.bus_type].push_back(path);
    } else if (bus_info.operation == BusOperation::kRead) {
      readers[bus_info.bus_type].push_back(path);
    }
  }

  // Validate: at most one writer per bus
  for (const auto& [bus, write_controls] : writers) {
    if (write_controls.size() > 1) {
      std::ostringstream message;
      message << "multiple writers to "
              << (bus == BusType::kData ? "data" : "address")
              << " bus in opcode " << opcode
              << " step " << step_index << ": ";
      for (size_t i = 0; i < write_controls.size(); ++i) {
        if (i > 0) message << ", ";
        message << write_controls[i];
      }
      throw MicrocodeError(message.str());
    }
  }

  // Validate: readers require a writer on the same bus
  for (const auto& [bus, read_controls] : readers) {
    if (writers.find(bus) == writers.end() || writers[bus].empty()) {
      std::ostringstream message;
      message << "readers on "
              << (bus == BusType::kData ? "data" : "address")
              << " bus without writer in opcode " << opcode
              << " step " << step_index << ": ";
      for (size_t i = 0; i < read_controls.size(); ++i) {
        if (i > 0) message << ", ";
        message << read_controls[i];
      }
      throw MicrocodeError(message.str());
    }
  }
}

void BusValidator::Run(ir::InstructionSet& instruction_set) const {
  // Validate fetch preamble
  for (size_t i = 0; i < instruction_set.fetch_preamble.size(); ++i) {
    ValidateStep(instruction_set.fetch_preamble[i], -1, static_cast<int>(i));
  }

  // Validate each instruction variant
  for (const auto& instruction : instruction_set.instructions) {
    for (const auto& variant : instruction.variants) {
      for (size_t i = 0; i < variant.steps.size(); ++i) {
        ValidateStep(variant.steps[i], static_cast<int>(instruction.opcode),
                     static_cast<int>(i));
      }
    }
  }
}

}  // namespace irata2::microcode::compiler

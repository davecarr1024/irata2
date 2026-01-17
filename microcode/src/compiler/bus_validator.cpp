#include "irata2/microcode/compiler/bus_validator.h"

#include "irata2/microcode/error.h"

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace irata2::microcode::compiler {

namespace {

enum class BusType {
  kData,
  kAddress,
  kNone  // For controls that don't use buses (halt, crash, etc.)
};

enum class BusOperation {
  kRead,
  kWrite,
  kNone  // For controls that don't perform bus operations
};

struct BusInfo {
  BusType bus_type;
  BusOperation operation;
};

// Determine which bus a control uses and whether it's reading or writing
BusInfo AnalyzeControl(std::string_view control_path) {
  // Extract component name (everything before the first dot or the whole path)
  size_t first_dot = control_path.find('.');
  std::string component;
  if (first_dot != std::string_view::npos) {
    component = std::string(control_path.substr(0, first_dot));
  } else {
    component = std::string(control_path);
  }

  // Determine bus type based on component
  BusType bus_type = BusType::kNone;

  // Address bus components
  if (component == "pc" || component == "tmp") {
    bus_type = BusType::kAddress;
  }
  // Data bus components
  else if (component == "a" || component == "x" || component == "alu" ||
           component == "status" || component == "controller") {
    bus_type = BusType::kData;
  }
  // Memory uses both buses (address for MAR word operations, data for everything else)
  else if (component == "memory") {
    // memory.mar.low and memory.mar.high use data bus (byte operations)
    // memory.mar (without .low/.high) uses address bus (word operations)
    // memory.read/write use data bus
    if (control_path.find("mar") != std::string_view::npos) {
      // Check if it's a byte operation (.low or .high) or word operation
      if (control_path.find("mar.low") != std::string_view::npos ||
          control_path.find("mar.high") != std::string_view::npos) {
        bus_type = BusType::kData;  // Byte operations use data bus
      } else {
        bus_type = BusType::kAddress;  // Word operations use address bus
      }
    } else {
      bus_type = BusType::kData;  // memory.read/write use data bus
    }
  }

  // Determine operation type
  BusOperation operation = BusOperation::kNone;
  if (control_path.find(".read") != std::string_view::npos) {
    operation = BusOperation::kRead;
  } else if (control_path.find(".write") != std::string_view::npos) {
    operation = BusOperation::kWrite;
  }

  return {bus_type, operation};
}

void ValidateStep(const ir::Step& step, int opcode, int step_index) {
  // Track writers and readers for each bus
  std::map<BusType, std::vector<std::string>> writers;
  std::map<BusType, std::vector<std::string>> readers;

  for (const auto* control : step.controls) {
    const auto bus_info = AnalyzeControl(control->path);

    if (bus_info.bus_type == BusType::kNone ||
        bus_info.operation == BusOperation::kNone) {
      // Control doesn't participate in bus operations (e.g., halt, crash)
      continue;
    }

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

}  // namespace

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

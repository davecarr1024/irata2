#include "irata2/base/log.h"
#include "irata2/hdl.h"
#include "irata2/microcode/compiler/compiler.h"
#include "irata2/microcode/debug/decoder.h"
#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/encoder/status_encoder.h"
#include "irata2/microcode/ir/irata_instruction_set.h"

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <iostream>
#include <optional>

ABSL_FLAG(std::string, format, "text",
          "Output format: 'text' or 'yaml' (default: text)");
ABSL_FLAG(std::optional<int>, opcode, std::nullopt,
          "Filter output to specific opcode (default: show all)");

namespace {

void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " [options]\n";
  std::cerr << "\n";
  std::cerr << "Options:\n";
  std::cerr << "  --format=<text|yaml>  Output format (default: text)\n";
  std::cerr << "  --opcode=<N>          Show only opcode N (default: show all)\n";
  std::cerr << "\n";
  std::cerr << "Dumps compiled microcode in human-readable format.\n";
}

}  // namespace

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  irata2::base::InitializeLogging();

  const std::string format = absl::GetFlag(FLAGS_format);
  const std::optional<int> opcode_filter = absl::GetFlag(FLAGS_opcode);

  if (format != "text" && format != "yaml") {
    std::cerr << "Error: Invalid format '" << format
              << "'. Must be 'text' or 'yaml'.\n\n";
    PrintUsage(argv[0]);
    return 1;
  }

  if (opcode_filter.has_value() &&
      (*opcode_filter < 0 || *opcode_filter > 255)) {
    std::cerr << "Error: Opcode must be in range 0-255.\n\n";
    PrintUsage(argv[0]);
    return 1;
  }

  try {
    // Build the default instruction set
    irata2::hdl::Cpu cpu;
    irata2::microcode::encoder::ControlEncoder control_encoder(cpu);

    // Build status bit definitions from CPU
    std::vector<irata2::microcode::output::StatusBitDefinition> status_bits = {
        {"carry", static_cast<uint8_t>(cpu.status().carry().bit_index())},
        {"zero", static_cast<uint8_t>(cpu.status().zero().bit_index())},
        {"interrupt_disable", static_cast<uint8_t>(cpu.status().interrupt_disable().bit_index())},
        {"decimal", static_cast<uint8_t>(cpu.status().decimal().bit_index())},
        {"break", static_cast<uint8_t>(cpu.status().brk().bit_index())},
        {"unused", static_cast<uint8_t>(cpu.status().unused().bit_index())},
        {"overflow", static_cast<uint8_t>(cpu.status().overflow().bit_index())},
        {"negative", static_cast<uint8_t>(cpu.status().negative().bit_index())},
    };

    irata2::microcode::encoder::StatusEncoder status_encoder(status_bits);
    irata2::microcode::compiler::Compiler compiler(
        control_encoder, status_encoder,
        cpu,
        cpu.controller().sc().increment().control_info(),
        cpu.controller().sc().reset().control_info());

    // Build the instruction set from the embedded microcode
    irata2::microcode::ir::InstructionSet instruction_set =
        irata2::microcode::ir::BuildIrataInstructionSet(cpu);

    // Compile to microcode program
    irata2::microcode::output::MicrocodeProgram program =
        compiler.Compile(instruction_set);

    // Create decoder
    irata2::microcode::debug::MicrocodeDecoder decoder(program);

    // Output based on format and filter
    std::string output;
    if (opcode_filter.has_value()) {
      if (format == "yaml") {
        output = decoder.DumpInstructionYaml(static_cast<uint8_t>(*opcode_filter));
      } else {
        output = decoder.DumpInstruction(static_cast<uint8_t>(*opcode_filter));
      }
    } else {
      if (format == "yaml") {
        output = decoder.DumpProgramYaml();
      } else {
        output = decoder.DumpProgram();
      }
    }

    std::cout << output;
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    return 1;
  }

  return 0;
}

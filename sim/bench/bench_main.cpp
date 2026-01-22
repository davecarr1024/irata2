#include "irata2/assembler/assembler.h"
#include "irata2/base/types.h"
#include "irata2/sim/cpu.h"
#include "irata2/sim/initialization.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {
struct Options {
  std::string workload = "loop";
  uint64_t cycles = 5'000'000;
  uint64_t warmup_cycles = 100'000;
};

void PrintUsage(const char* argv0) {
  std::cerr << "Usage: " << argv0
            << " [--workload {loop,mem}] [--cycles N] [--warmup N]\n";
}

std::optional<uint64_t> ParseU64(const std::string& value) {
  try {
    size_t idx = 0;
    uint64_t parsed = std::stoull(value, &idx);
    if (idx != value.size()) {
      return std::nullopt;
    }
    return parsed;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

std::string LoopProgram() {
  return R"(
    CLC
    LDA #$00
  loop:
    ADC #$01
    CMP #$FF
    BNE loop
    JMP loop
  )";
}

std::string MemProgram() {
  return R"(
    LDX #$00
  loop:
    LDA $0200,X
    STA $0300,X
    INX
    BNE loop
    JMP loop
  )";
}

irata2::sim::Cpu MakeCpu(std::string_view asm_source) {
  const auto assembled = irata2::assembler::Assemble(asm_source, "bench.asm");
  std::vector<irata2::base::Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(irata2::base::Byte{value});
  }

  irata2::sim::Cpu cpu(irata2::sim::DefaultHdl(),
                       irata2::sim::DefaultMicrocodeProgram(),
                       std::move(rom));
  cpu.pc().set_value(irata2::base::Word{0x8000});
  cpu.controller().sc().set_value(irata2::base::Byte{0});
  cpu.controller().ir().set_value(cpu.memory().ReadAt(irata2::base::Word{0x8000}));
  return cpu;
}

const char* HaltReasonToString(irata2::sim::Cpu::HaltReason reason) {
  switch (reason) {
    case irata2::sim::Cpu::HaltReason::Running:
      return "running";
    case irata2::sim::Cpu::HaltReason::Timeout:
      return "timeout";
    case irata2::sim::Cpu::HaltReason::Halt:
      return "halt";
    case irata2::sim::Cpu::HaltReason::Crash:
      return "crash";
  }
  return "unknown";
}

void RunBenchmark(const Options& options) {
  const std::string program = (options.workload == "mem") ? MemProgram()
                                                           : LoopProgram();

  if (options.warmup_cycles > 0) {
    auto warmup_cpu = MakeCpu(program);
    warmup_cpu.RunUntilHalt(options.warmup_cycles);
  }

  auto cpu = MakeCpu(program);
  const auto start = std::chrono::steady_clock::now();
  const auto result = cpu.RunUntilHalt(options.cycles);
  const auto end = std::chrono::steady_clock::now();

  const std::chrono::duration<double> elapsed = end - start;
  const double seconds = elapsed.count();
  const double cycles = static_cast<double>(result.cycles);
  const double cycles_per_sec = seconds > 0.0 ? cycles / seconds : 0.0;

  std::cout << "workload=" << options.workload
            << " cycles=" << result.cycles
            << " elapsed_s=" << seconds
            << " cycles_per_sec=" << cycles_per_sec
            << " halt_reason=" << HaltReasonToString(result.reason)
            << "\n";
}
}  // namespace

int main(int argc, char** argv) {
  Options options;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--workload") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      options.workload = argv[++i];
      if (options.workload != "loop" && options.workload != "mem") {
        std::cerr << "Unknown workload: " << options.workload << "\n";
        PrintUsage(argv[0]);
        return 1;
      }
      continue;
    }
    if (arg == "--cycles") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      auto parsed = ParseU64(argv[++i]);
      if (!parsed) {
        std::cerr << "Invalid cycles value\n";
        return 1;
      }
      options.cycles = *parsed;
      continue;
    }
    if (arg == "--warmup") {
      if (i + 1 >= argc) {
        PrintUsage(argv[0]);
        return 1;
      }
      auto parsed = ParseU64(argv[++i]);
      if (!parsed) {
        std::cerr << "Invalid warmup value\n";
        return 1;
      }
      options.warmup_cycles = *parsed;
      continue;
    }
    PrintUsage(argv[0]);
    return 1;
  }

  RunBenchmark(options);
  return 0;
}

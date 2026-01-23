#include "irata2/assembler/assembler.h"
#include "irata2/sim.h"
#include "irata2/sim/io/input_device.h"

#include <gtest/gtest.h>

#include <memory>

using irata2::assembler::AssemblerResult;
using irata2::assembler::Assemble;
using irata2::base::Byte;
using irata2::base::Word;
using irata2::sim::Cpu;
using irata2::sim::DefaultHdl;
using irata2::sim::DefaultMicrocodeProgram;
using irata2::sim::io::InputDevice;
using irata2::sim::io::INPUT_DEVICE_BASE;
using irata2::sim::memory::Memory;
using irata2::sim::memory::Region;

namespace {
struct CpuWithInput {
  std::unique_ptr<Cpu> cpu;
  InputDevice* device = nullptr;
};

CpuWithInput MakeCpuWithInputDevice(const std::vector<Byte>& rom) {
  CpuWithInput result;

  std::vector<Memory::RegionFactory> factories;
  factories.push_back([&result](Memory& mem) -> std::unique_ptr<Region> {
    return std::make_unique<Region>(
        "input_device", mem, Word{INPUT_DEVICE_BASE},
        [&result](Region& region) -> std::unique_ptr<irata2::sim::memory::Module> {
          auto device = std::make_unique<InputDevice>("input", region);
          result.device = device.get();
          return device;
        });
  });

  result.cpu = std::make_unique<Cpu>(
      DefaultHdl(), DefaultMicrocodeProgram(), rom, std::move(factories));
  return result;
}

void InitializeCpu(Cpu& cpu, Word entry) {
  cpu.pc().set_value(entry);
  cpu.controller().sc().set_value(Byte{0});
  cpu.controller().ir().set_value(cpu.memory().ReadAt(entry));
}
}  // namespace

TEST(InputDeviceIntegrationTest, ProgramReadsInputAndStoresToRam) {
  const std::string program = R"(
    LDA $4000
    AND #$01
    BEQ wait
    LDA $4002
    STA $0200
    HLT
  wait:
    JMP wait
  )";

  AssemblerResult assembled = Assemble(program, "input_device_integration.asm");
  std::vector<Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(Byte{value});
  }

  CpuWithInput rig = MakeCpuWithInputDevice(rom);
  ASSERT_NE(rig.device, nullptr);

  rig.device->inject_key(0x41);
  InitializeCpu(*rig.cpu, assembled.header.entry);

  auto result = rig.cpu->RunUntilHalt(2000);
  ASSERT_EQ(result.reason, Cpu::HaltReason::Halt);

  Byte stored = rig.cpu->memory().ReadAt(Word{0x0200});
  EXPECT_EQ(stored.value(), 0x41);
}

#include "irata2/assembler/assembler.h"
#include "irata2/sim.h"
#include "irata2/sim/io/vector_graphics_coprocessor.h"
#include "irata2/sim/io/vgc_backend.h"

#include <gtest/gtest.h>

#include <memory>

using irata2::assembler::Assemble;
using irata2::assembler::AssemblerResult;
using irata2::base::Byte;
using irata2::base::Word;
using irata2::sim::Cpu;
using irata2::sim::DefaultHdl;
using irata2::sim::DefaultMicrocodeProgram;
using irata2::sim::LatchedProcessControl;
using irata2::sim::io::ImageBackend;
using irata2::sim::io::VectorGraphicsCoprocessor;
using irata2::sim::io::VGC_BASE;
using irata2::sim::memory::Memory;
using irata2::sim::memory::Region;

namespace {
struct VgcRig {
  std::unique_ptr<Cpu> cpu;
  VectorGraphicsCoprocessor* vgc = nullptr;
  ImageBackend* backend = nullptr;
};

VgcRig MakeCpuWithVgc(const std::vector<Byte>& rom) {
  VgcRig rig;
  std::vector<Memory::RegionFactory> factories;
  factories.push_back([&rig](Memory& mem,
                             LatchedProcessControl&)
                           -> std::unique_ptr<Region> {
    return std::make_unique<Region>(
        "vgc", mem, Word{VGC_BASE},
        [&rig](Region& region) -> std::unique_ptr<irata2::sim::memory::Module> {
          auto backend = std::make_unique<ImageBackend>();
          rig.backend = backend.get();
          auto vgc = std::make_unique<VectorGraphicsCoprocessor>(
              "vgc", region, std::move(backend));
          rig.vgc = vgc.get();
          return vgc;
        });
  });

  rig.cpu = std::make_unique<Cpu>(
      DefaultHdl(), DefaultMicrocodeProgram(), rom, std::move(factories));
  return rig;
}

void InitializeCpu(Cpu& cpu, Word entry) {
  cpu.pc().set_value(entry);
  cpu.controller().sc().set_value(Byte{0});
  cpu.controller().ir().set_value(cpu.memory().ReadAt(entry));
}
}  // namespace

TEST(VgcIntegrationTest, ProgramDrawsLineToFramebuffer) {
  const std::string program = R"(
    LDA #$01
    STA $4100
    LDA #$03
    STA $4105
    LDA #$01
    STA $4106

    LDA #$03
    STA $4100
    LDA #$00
    STA $4101
    LDA #$00
    STA $4102
    LDA #$0A
    STA $4103
    LDA #$0A
    STA $4104
    LDA #$03
    STA $4105
    LDA #$01
    STA $4106

    LDA #$02
    STA $4107
    HLT
  )";

  AssemblerResult assembled = Assemble(program, "vgc_integration.asm");
  std::vector<Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(Byte{value});
  }

  VgcRig rig = MakeCpuWithVgc(rom);
  ASSERT_NE(rig.backend, nullptr);

  InitializeCpu(*rig.cpu, assembled.header.entry);
  auto result = rig.cpu->RunUntilHalt(3000);
  ASSERT_EQ(result.reason, Cpu::HaltReason::Halt);

  const auto& fb = rig.backend->framebuffer();
  EXPECT_EQ(fb[0], 0x03);
  EXPECT_EQ(fb[10 * ImageBackend::kWidth + 10], 0x03);
}

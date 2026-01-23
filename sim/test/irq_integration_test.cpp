#include "irata2/assembler/assembler.h"
#include "irata2/sim.h"
#include "irata2/sim/control.h"
#include "irata2/sim/memory/module.h"
#include "test_helpers.h"

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
using irata2::sim::memory::Memory;
using irata2::sim::memory::Module;
using irata2::sim::memory::Region;

namespace {
class TestIrqDevice final : public Module {
 public:
  static constexpr size_t kSize = 16;

  TestIrqDevice(std::string name,
                Component& parent,
                LatchedProcessControl& irq_line)
      : Module(std::move(name), parent),
        irq_line_(irq_line) {}

  size_t size() const override { return kSize; }

  Byte Read(Word address) const override {
    switch (address.value()) {
      case 0x00:
        return pending_ ? Byte{0x01} : Byte{0x00};
      case 0x01:
        const_cast<TestIrqDevice*>(this)->pending_ = false;
        return Byte{0xAA};
      default:
        return Byte{0x00};
    }
  }

  void Write(Word, Byte) override {}

  void Trigger() { pending_ = true; }
  bool pending() const { return pending_; }

  void TickControl() override {
    irq_line_.Set(pending_);
  }

 private:
  LatchedProcessControl& irq_line_;
  bool pending_ = false;
};

struct IrqRig {
  std::unique_ptr<Cpu> cpu;
  TestIrqDevice* device = nullptr;
};

IrqRig MakeCpuWithIrqDevice(const std::vector<Byte>& rom) {
  IrqRig rig;
  std::vector<Memory::RegionFactory> factories;
  factories.push_back([&rig](Memory& mem, LatchedProcessControl& irq_line)
                          -> std::unique_ptr<Region> {
    return std::make_unique<Region>(
        "irq_device", mem, Word{0x5000},
        [&rig, &irq_line](Region& region) -> std::unique_ptr<Module> {
          auto device = std::make_unique<TestIrqDevice>("irq", region, irq_line);
          rig.device = device.get();
          return device;
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

TEST(IrqIntegrationTest, DeviceTriggersHandlerAndRtiReturns) {
  const std::string program = R"(
    .org $8000
  main:
    INC $0000
    JMP main

    .org $9000
  irq_handler:
    LDA $5001
    INC $0001
    RTI

    .org $FFFE
    .byte $00, $90
  )";

  AssemblerResult assembled = Assemble(program, "irq_integration.asm");
  std::vector<Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(Byte{value});
  }

  IrqRig rig = MakeCpuWithIrqDevice(rom);
  ASSERT_NE(rig.device, nullptr);

  InitializeCpu(*rig.cpu, assembled.header.entry);
  rig.cpu->RunUntilHalt(200);
  const Byte count_before = rig.cpu->memory().ReadAt(Word{0x0000});

  rig.device->Trigger();
  rig.cpu->RunUntilHalt(2000);

  const Byte irq_count = rig.cpu->memory().ReadAt(Word{0x0001});
  EXPECT_GT(irq_count.value(), 0);
  const Byte count_after = rig.cpu->memory().ReadAt(Word{0x0000});
  EXPECT_GE(count_after.value(), count_before.value());
  EXPECT_FALSE(rig.device->pending());

  rig.cpu->Tick();
  irata2::sim::test::SetPhase(*rig.cpu, irata2::base::TickPhase::Process);
  EXPECT_FALSE(rig.cpu->irq_line().asserted());
}

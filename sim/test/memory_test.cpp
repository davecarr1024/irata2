#include "irata2/sim.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;
using namespace irata2::sim::memory;

namespace {
// Simple test fixture for memory component tests
class TestCpu;
class TestParent : public Component {
 public:
  TestParent() {}

  Cpu& cpu() override {
    throw SimError("TestParent::cpu() not supported");
  }

  const Cpu& cpu() const override {
    throw SimError("TestParent::cpu() not supported");
  }

  std::string path() const override { return "test_parent"; }

  irata2::base::TickPhase current_phase() const override {
    return irata2::base::TickPhase::Clear;
  }
};
}  // namespace

TEST(SimMemoryModuleTest, RamReadWrite) {
  TestParent parent;
  Ram ram("ram", parent, 4, irata2::base::Byte{0x00});
  ram.Write(irata2::base::Word{1}, irata2::base::Byte{0xAA});
  EXPECT_EQ(ram.Read(irata2::base::Word{1}), irata2::base::Byte{0xAA});
}

TEST(SimMemoryModuleTest, RomRejectsWrite) {
  TestParent parent;
  Rom rom("rom", parent, 4, irata2::base::Byte{0xFF});
  EXPECT_THROW(rom.Write(irata2::base::Word{0}, irata2::base::Byte{0x11}),
               SimError);
}

TEST(SimMemoryRegionTest, RejectsNonPowerOfTwoSize) {
  TestParent parent;
  EXPECT_THROW((Region("bad", parent, irata2::base::Word{0},
                       [](Region& r) -> std::unique_ptr<Module> {
                         return std::make_unique<Ram>("ram", r, 3,
                                                       irata2::base::Byte{0});
                       })),
               SimError);
}

TEST(SimMemoryRegionTest, RejectsMisalignedOffset) {
  TestParent parent;
  EXPECT_THROW((Region("bad", parent, irata2::base::Word{2},
                       [](Region& r) -> std::unique_ptr<Module> {
                         return std::make_unique<Ram>("ram", r, 4,
                                                       irata2::base::Byte{0});
                       })),
               SimError);
}

TEST(SimMemoryTest, RejectsOverlappingRegions) {
  Cpu sim = test::MakeTestCpu();
  std::vector<Memory::RegionFactory> region_factories;
  region_factories.push_back([](Memory& m,
                                LatchedProcessControl&)
                                  -> std::unique_ptr<Region> {
    return std::make_unique<Region>(
        "one", m, irata2::base::Word{0x0000},
        [](Region& r) -> std::unique_ptr<Module> {
          return std::make_unique<Ram>("ram", r, 0x2000,
                                        irata2::base::Byte{0});
        });
  });
  region_factories.push_back([](Memory& m,
                                LatchedProcessControl&)
                                  -> std::unique_ptr<Region> {
    return std::make_unique<Region>(
        "two", m, irata2::base::Word{0x1000},
        [](Region& r) -> std::unique_ptr<Module> {
          return std::make_unique<Ram>("ram", r, 0x1000,
                                        irata2::base::Byte{0});
        });
  });

  EXPECT_THROW(Memory("memory", sim, sim.data_bus(), sim.address_bus(),
                      std::move(region_factories), sim.irq_line()),
               SimError);
}

TEST(SimMemoryTest, ReadUnmappedReturnsFF) {
  Cpu sim = test::MakeTestCpu();
  const auto value = sim.memory().ReadAt(irata2::base::Word{0x4000});
  EXPECT_EQ(value, irata2::base::Byte{0xFF});
}

TEST(SimMemoryTest, WriteUnmappedThrows) {
  Cpu sim = test::MakeTestCpu();
  EXPECT_THROW(sim.memory().WriteAt(irata2::base::Word{0x4000},
                                    irata2::base::Byte{0x12}),
               SimError);
}

TEST(SimMemoryTest, WritesThroughBusToRam) {
  Cpu sim = test::MakeTestCpu();

  sim.memory().mar().set_value(irata2::base::Word{0x0001});
  sim.a().set_value(irata2::base::Byte{0x7E});
  test::AssertControl(sim.a().write());
  test::AssertControl(sim.memory().read());
  sim.Tick();

  EXPECT_EQ(sim.memory().ReadAt(irata2::base::Word{0x0001}),
            irata2::base::Byte{0x7E});
}

TEST(SimMemoryTest, ReadsThroughBusFromRam) {
  Cpu sim = test::MakeTestCpu();

  sim.memory().mar().set_value(irata2::base::Word{0x0002});
  sim.memory().WriteAt(irata2::base::Word{0x0002},
                       irata2::base::Byte{0x3C});
  test::AssertControl(sim.memory().write());
  test::AssertControl(sim.x().read());
  sim.Tick();

  EXPECT_EQ(sim.x().value(), irata2::base::Byte{0x3C});
}

TEST(SimMemoryAddressRegisterTest, ReadsWordFromAddressBus) {
  Cpu sim = test::MakeTestCpu();

  sim.pc().set_value(irata2::base::Word{0x1234});
  test::AssertControl(sim.pc().write());
  test::AssertControl(sim.memory().mar().read());
  sim.Tick();

  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0x1234});
}

TEST(SimMemoryAddressRegisterTest, ReadsLowHighFromDataBus) {
  Cpu sim = test::MakeTestCpu();

  sim.a().set_value(irata2::base::Byte{0xCD});
  test::AssertControl(sim.a().write());
  test::AssertControl(sim.memory().mar().low().read());
  sim.Tick();

  sim.a().set_value(irata2::base::Byte{0xAB});
  test::AssertControl(sim.a().write());
  test::AssertControl(sim.memory().mar().high().read());
  sim.Tick();

  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0xABCD});
}

TEST(SimMemoryAddressRegisterTest, AddOffsetWithoutCarry) {
  Cpu sim = test::MakeTestCpu();

  // Set MAR to 0x1234, offset to 0x05
  sim.memory().mar().set_value(irata2::base::Word{0x1234});
  sim.memory().mar().offset().set_value(irata2::base::Byte{0x05});

  // Assert add_offset control
  test::AssertControl(sim.memory().mar().add_offset());
  sim.Tick();

  // Result should be 0x1234 + 0x05 = 0x1239
  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0x1239});
}

TEST(SimMemoryAddressRegisterTest, AddOffsetWithCarry) {
  Cpu sim = test::MakeTestCpu();

  // Set MAR to 0x12FE, offset to 0x05 - will overflow low byte
  sim.memory().mar().set_value(irata2::base::Word{0x12FE});
  sim.memory().mar().offset().set_value(irata2::base::Byte{0x05});

  // Assert add_offset control
  test::AssertControl(sim.memory().mar().add_offset());
  sim.Tick();

  // Result should be 0x12FE + 0x05 = 0x1303 (carry to high byte)
  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0x1303});
}

TEST(SimMemoryAddressRegisterTest, AddOffsetZeroPageWrap) {
  Cpu sim = test::MakeTestCpu();

  // Set MAR to 0x00FE (zero page), offset to 0x05
  sim.memory().mar().set_value(irata2::base::Word{0x00FE});
  sim.memory().mar().offset().set_value(irata2::base::Byte{0x05});

  // Assert add_offset control
  test::AssertControl(sim.memory().mar().add_offset());
  sim.Tick();

  // Without reset, result would be 0x0103 (with carry)
  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0x0103});

  // Reset high byte to simulate zero page wrap
  test::AssertControl(sim.memory().mar().high().reset());
  sim.Tick();

  // Now should be 0x0003 (wrapped in zero page)
  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0x0003});
}

TEST(SimMemoryAddressRegisterTest, AddOffsetLoadsFromDataBus) {
  Cpu sim = test::MakeTestCpu();

  // Set MAR to some value
  sim.memory().mar().set_value(irata2::base::Word{0x1020});

  // Load offset from X register via data bus
  sim.x().set_value(irata2::base::Byte{0x0A});
  test::AssertControl(sim.x().write());
  test::AssertControl(sim.memory().mar().offset().read());
  sim.Tick();

  // Verify offset was loaded
  EXPECT_EQ(sim.memory().mar().offset().value(), irata2::base::Byte{0x0A});

  // Now add it
  test::AssertControl(sim.memory().mar().add_offset());
  sim.Tick();

  // Result should be 0x1020 + 0x0A = 0x102A
  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0x102A});
}

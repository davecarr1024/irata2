#include "irata2/sim.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;
using namespace irata2::sim::memory;

TEST(SimMemoryModuleTest, RamReadWrite) {
  Ram ram(4, irata2::base::Byte{0x00});
  ram.Write(irata2::base::Word{1}, irata2::base::Byte{0xAA});
  EXPECT_EQ(ram.Read(irata2::base::Word{1}), irata2::base::Byte{0xAA});
}

TEST(SimMemoryModuleTest, RomRejectsWrite) {
  Rom rom(4, irata2::base::Byte{0xFF});
  EXPECT_THROW(rom.Write(irata2::base::Word{0}, irata2::base::Byte{0x11}),
               SimError);
}

TEST(SimMemoryRegionTest, RejectsNonPowerOfTwoSize) {
  auto module = MakeRam(3);
  EXPECT_THROW(Region("bad", irata2::base::Word{0}, module), SimError);
}

TEST(SimMemoryRegionTest, RejectsMisalignedOffset) {
  auto module = MakeRam(4);
  EXPECT_THROW(Region("bad", irata2::base::Word{2}, module), SimError);
}

TEST(SimMemoryTest, RejectsOverlappingRegions) {
  Cpu sim = test::MakeTestCpu();
  std::vector<Region> regions;
  regions.emplace_back("one", irata2::base::Word{0x0000}, MakeRam(0x2000));
  regions.emplace_back("two", irata2::base::Word{0x1000}, MakeRam(0x1000));

  EXPECT_THROW(Memory("memory", sim, sim.data_bus(), sim.address_bus(),
                      std::move(regions)),
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

#include "irata2/sim.h"

#include <gtest/gtest.h>

using namespace irata2::sim;
using namespace irata2::sim::memory;

namespace {
void SetSafeIr(Cpu& sim) {
  sim.controller().ir().set_value(irata2::base::Byte{0x02});
  sim.controller().sc().set_value(irata2::base::Byte{0});
}
}  // namespace

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
  Cpu sim;
  std::vector<Region> regions;
  regions.emplace_back("one", irata2::base::Word{0x0000}, MakeRam(0x2000));
  regions.emplace_back("two", irata2::base::Word{0x1000}, MakeRam(0x1000));

  EXPECT_THROW(Memory("memory", sim, sim.data_bus(), sim.address_bus(),
                      std::move(regions)),
               SimError);
}

TEST(SimMemoryTest, ReadUnmappedReturnsFF) {
  Cpu sim;
  const auto value = sim.memory().ReadAt(irata2::base::Word{0x4000});
  EXPECT_EQ(value, irata2::base::Byte{0xFF});
}

TEST(SimMemoryTest, WriteUnmappedThrows) {
  Cpu sim;
  EXPECT_THROW(sim.memory().WriteAt(irata2::base::Word{0x4000},
                                    irata2::base::Byte{0x12}),
               SimError);
}

TEST(SimMemoryTest, WritesThroughBusToRam) {
  Cpu sim;

  SetSafeIr(sim);
  sim.memory().mar().set_value(irata2::base::Word{0x0001});
  sim.a().set_value(irata2::base::Byte{0x7E});
  sim.a().write().Assert();
  sim.memory().read().Assert();
  sim.Tick();

  EXPECT_EQ(sim.memory().ReadAt(irata2::base::Word{0x0001}),
            irata2::base::Byte{0x7E});
}

TEST(SimMemoryTest, ReadsThroughBusFromRam) {
  Cpu sim;

  SetSafeIr(sim);
  sim.memory().mar().set_value(irata2::base::Word{0x0002});
  sim.memory().WriteAt(irata2::base::Word{0x0002},
                       irata2::base::Byte{0x3C});
  sim.memory().write().Assert();
  sim.x().read().Assert();
  sim.Tick();

  EXPECT_EQ(sim.x().value(), irata2::base::Byte{0x3C});
}

TEST(SimMemoryAddressRegisterTest, ReadsWordFromAddressBus) {
  Cpu sim;

  SetSafeIr(sim);
  sim.pc().set_value(irata2::base::Word{0x1234});
  sim.pc().write().Assert();
  sim.memory().mar().read().Assert();
  sim.Tick();

  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0x1234});
}

TEST(SimMemoryAddressRegisterTest, ReadsLowHighFromDataBus) {
  Cpu sim;

  SetSafeIr(sim);
  sim.a().set_value(irata2::base::Byte{0xCD});
  sim.a().write().Assert();
  sim.memory().mar().low().read().Assert();
  sim.Tick();

  sim.a().set_value(irata2::base::Byte{0xAB});
  sim.a().write().Assert();
  sim.memory().mar().high().read().Assert();
  sim.Tick();

  EXPECT_EQ(sim.memory().mar().value(), irata2::base::Word{0xABCD});
}

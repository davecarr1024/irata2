#include "irata2/base/tick_phase.h"

#include <gtest/gtest.h>

using namespace irata2::base;

TEST(TickPhaseTest, ToStringNone) {
  EXPECT_EQ(ToString(TickPhase::None), "None");
}

TEST(TickPhaseTest, ToStringControl) {
  EXPECT_EQ(ToString(TickPhase::Control), "Control");
}

TEST(TickPhaseTest, ToStringWrite) {
  EXPECT_EQ(ToString(TickPhase::Write), "Write");
}

TEST(TickPhaseTest, ToStringRead) {
  EXPECT_EQ(ToString(TickPhase::Read), "Read");
}

TEST(TickPhaseTest, ToStringProcess) {
  EXPECT_EQ(ToString(TickPhase::Process), "Process");
}

TEST(TickPhaseTest, ToStringClear) {
  EXPECT_EQ(ToString(TickPhase::Clear), "Clear");
}

TEST(TickPhaseTest, ToStringUnknown) {
  auto unknown = static_cast<TickPhase>(0x7F);
  EXPECT_EQ(ToString(unknown), "Unknown");
}

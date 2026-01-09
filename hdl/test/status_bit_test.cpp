#include "irata2/hdl.h"
#include "irata2/hdl/status.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(StatusTest, ExposesRegisterAndBitIndex) {
  Cpu cpu;

  EXPECT_EQ(cpu.status().zero().bit_index(), 1);
  EXPECT_EQ(cpu.status().carry().bit_index(), 0);
  EXPECT_EQ(cpu.status().negative().bit_index(), 7);
}

TEST(StatusTest, PathsUseStatusRegisterNames) {
  Cpu cpu;
  EXPECT_EQ(cpu.status().path(), "status");
  EXPECT_EQ(cpu.status().zero().path(), "status.zero");
  EXPECT_EQ(cpu.status().negative().path(), "status.negative");
}

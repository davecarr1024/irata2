#include "irata2/assembler/assembler.h"

#include <gtest/gtest.h>

using irata2::assembler::AssembleFile;

namespace {
std::string DemoPath(const char* filename) {
  return std::string(IRATA2_TEST_SOURCE_DIR) + "/demos/" + filename;
}
}  // namespace

TEST(DemoProgramsTest, AssemblesBlinkDemo) {
  auto result = AssembleFile(DemoPath("blink.asm"));
  EXPECT_FALSE(result.rom.empty());
}

TEST(DemoProgramsTest, AssemblesMoveSpriteDemo) {
  auto result = AssembleFile(DemoPath("move_sprite.asm"));
  EXPECT_FALSE(result.rom.empty());
}

TEST(DemoProgramsTest, AssemblesAsteroidsDemo) {
  auto result = AssembleFile(DemoPath("asteroids.asm"));
  EXPECT_FALSE(result.rom.empty());
}

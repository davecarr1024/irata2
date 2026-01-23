#include <gtest/gtest.h>

#include "graphics_test_helpers.h"

namespace irata2::assembler::test {

TEST(GraphicsIntegration, ClearScreen) {
  const char* source = R"(
    .org $8000
    LDA #$01        ; CLEAR command
    STA $4100
    LDA #$00        ; Black
    STA $4105
    LDA #$01        ; Execute
    STA $4106
    HLT
  )";

  auto fb = ExecuteAndRender(source);
  ExpectClear(fb, 0);  // All pixels should be black
  EXPECT_EQ(CountPixels(fb, 0), 256 * 256);
}

TEST(GraphicsIntegration, DrawPoint) {
  const char* source = R"(
    .org $8000
    ; Clear to black first
    LDA #$01
    STA $4100
    LDA #$00
    STA $4105
    LDA #$01
    STA $4106

    ; Draw point at (128, 144)
    LDA #$02        ; POINT command
    STA $4100
    LDA #$80        ; X=128
    STA $4101
    LDA #$90        ; Y=144
    STA $4102
    LDA #$03        ; White
    STA $4105
    LDA #$01        ; Execute
    STA $4106
    HLT
  )";

  auto fb = ExecuteAndRender(source);
  ExpectPixel(fb, 128, 144, 3);  // White pixel at (128, 144)
  EXPECT_EQ(CountPixels(fb, 3), 1);  // Only one white pixel
  EXPECT_EQ(CountPixels(fb, 0), 256 * 256 - 1);  // Rest are black
}

TEST(GraphicsIntegration, DrawLineHorizontal) {
  const char* source = R"(
    .org $8000
    ; Clear to black
    LDA #$01
    STA $4100
    LDA #$00
    STA $4105
    LDA #$01
    STA $4106

    ; Draw horizontal line from (10, 50) to (20, 50)
    LDA #$03        ; LINE command
    STA $4100
    LDA #$0A        ; X0=10
    STA $4101
    LDA #$32        ; Y0=50
    STA $4102
    LDA #$14        ; X1=20
    STA $4103
    LDA #$32        ; Y1=50
    STA $4104
    LDA #$03        ; White
    STA $4105
    LDA #$01        ; Execute
    STA $4106
    HLT
  )";

  auto fb = ExecuteAndRender(source);

  // Check all pixels on the line
  for (uint8_t x = 10; x <= 20; ++x) {
    ExpectPixel(fb, x, 50, 3);
  }

  // Line should be 11 pixels (10-20 inclusive)
  EXPECT_EQ(CountPixels(fb, 3), 11);
}

TEST(GraphicsIntegration, DrawLineVertical) {
  const char* source = R"(
    .org $8000
    ; Clear to black
    LDA #$01
    STA $4100
    LDA #$00
    STA $4105
    LDA #$01
    STA $4106

    ; Draw vertical line from (100, 30) to (100, 40)
    LDA #$03        ; LINE command
    STA $4100
    LDA #$64        ; X0=100
    STA $4101
    LDA #$1E        ; Y0=30
    STA $4102
    LDA #$64        ; X1=100
    STA $4103
    LDA #$28        ; Y1=40
    STA $4104
    LDA #$02        ; Medium green
    STA $4105
    LDA #$01        ; Execute
    STA $4106
    HLT
  )";

  auto fb = ExecuteAndRender(source);

  // Check all pixels on the line
  for (uint8_t y = 30; y <= 40; ++y) {
    ExpectPixel(fb, 100, y, 2);
  }

  // Line should be 11 pixels (30-40 inclusive)
  EXPECT_EQ(CountPixels(fb, 2), 11);
}

TEST(GraphicsIntegration, UseNamedConstants) {
  const char* source = R"(
    ; Define VGC constants
    .equ VGC_CMD, $4100
    .equ VGC_X0, $4101
    .equ VGC_Y0, $4102
    .equ VGC_COLOR, $4105
    .equ VGC_EXEC, $4106

    .equ CMD_CLEAR, $01
    .equ CMD_POINT, $02
    .equ COLOR_BLACK, $00
    .equ COLOR_WHITE, $03

    .org $8000
    ; Clear screen using constants
    LDA #CMD_CLEAR
    STA VGC_CMD
    LDA #COLOR_BLACK
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC

    ; Draw point using constants
    LDA #CMD_POINT
    STA VGC_CMD
    LDA #$50
    STA VGC_X0
    LDA #$50
    STA VGC_Y0
    LDA #COLOR_WHITE
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    HLT
  )";

  auto fb = ExecuteAndRender(source);
  ExpectPixel(fb, 80, 80, 3);  // (0x50, 0x50) = (80, 80)
  EXPECT_EQ(CountPixels(fb, 3), 1);
}

}  // namespace irata2::assembler::test

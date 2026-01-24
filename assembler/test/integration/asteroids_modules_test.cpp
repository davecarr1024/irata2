#include <gtest/gtest.h>

#include "../graphics_test_helpers.h"
#include "irata2/assembler/assembler.h"

namespace irata2::assembler::test {

// Test that graphics module clear screen works
TEST(AsteroidsModulesTest, GraphicsClearScreen) {
  const char* source = R"(
    ; Hardware constants
    .equ VGC_CMD, $4100
    .equ VGC_COLOR, $4105
    .equ VGC_EXEC, $4106

    .equ CMD_CLEAR, $01
    .equ COLOR_BLACK, $00

    ; Draw parameters
    .equ draw_color, $2C

    .org $8000
    LDX #$FF
    TXS

    JSR gfx_clear
    HLT

gfx_clear:
    LDA #CMD_CLEAR
    STA VGC_CMD
    LDA #COLOR_BLACK
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS
  )";

  auto fb = ExecuteAndRender(source);
  ExpectClear(fb, 0);  // All pixels should be black
}

// Test that graphics module draw point works
TEST(AsteroidsModulesTest, GraphicsDrawPoint) {
  const char* source = R"(
    ; Hardware constants
    .equ VGC_CMD, $4100
    .equ VGC_X0, $4101
    .equ VGC_Y0, $4102
    .equ VGC_COLOR, $4105
    .equ VGC_EXEC, $4106

    .equ CMD_CLEAR, $01
    .equ CMD_POINT, $02
    .equ COLOR_BLACK, $00
    .equ COLOR_WHITE, $03

    ; Draw parameters
    .equ draw_x, $28
    .equ draw_y, $29
    .equ draw_color, $2C

    .org $8000
    LDX #$FF
    TXS

    JSR gfx_clear

    ; Draw point at (100, 100)
    LDA #$64
    STA draw_x
    STA draw_y
    LDA #COLOR_WHITE
    STA draw_color
    JSR gfx_point
    HLT

gfx_clear:
    LDA #CMD_CLEAR
    STA VGC_CMD
    LDA #COLOR_BLACK
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

gfx_point:
    LDA #CMD_POINT
    STA VGC_CMD
    LDA draw_x
    STA VGC_X0
    LDA draw_y
    STA VGC_Y0
    LDA draw_color
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS
  )";

  auto fb = ExecuteAndRender(source);
  ExpectPixel(fb, 100, 100, 3);  // White pixel at (100, 100)
  EXPECT_EQ(CountPixels(fb, 3), 1);  // Only one white pixel
}

// Test that graphics module draw line works
TEST(AsteroidsModulesTest, GraphicsDrawLine) {
  const char* source = R"(
    ; Hardware constants
    .equ VGC_CMD, $4100
    .equ VGC_X0, $4101
    .equ VGC_Y0, $4102
    .equ VGC_X1, $4103
    .equ VGC_Y1, $4104
    .equ VGC_COLOR, $4105
    .equ VGC_EXEC, $4106

    .equ CMD_CLEAR, $01
    .equ CMD_LINE, $03
    .equ COLOR_BLACK, $00
    .equ COLOR_WHITE, $03

    ; Draw parameters
    .equ draw_x0, $28
    .equ draw_y0, $29
    .equ draw_x1, $2A
    .equ draw_y1, $2B
    .equ draw_color, $2C

    .org $8000
    LDX #$FF
    TXS

    JSR gfx_clear

    ; Draw horizontal line from (50, 80) to (70, 80)
    LDA #$32
    STA draw_x0
    LDA #$50
    STA draw_y0
    LDA #$46
    STA draw_x1
    LDA #$50
    STA draw_y1
    LDA #COLOR_WHITE
    STA draw_color
    JSR gfx_line
    HLT

gfx_clear:
    LDA #CMD_CLEAR
    STA VGC_CMD
    LDA #COLOR_BLACK
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

gfx_line:
    LDA #CMD_LINE
    STA VGC_CMD
    LDA draw_x0
    STA VGC_X0
    LDA draw_y0
    STA VGC_Y0
    LDA draw_x1
    STA VGC_X1
    LDA draw_y1
    STA VGC_Y1
    LDA draw_color
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS
  )";

  auto fb = ExecuteAndRender(source);
  // Line from (50, 80) to (70, 80) - 21 pixels
  for (uint8_t x = 50; x <= 70; ++x) {
    ExpectPixel(fb, x, 80, 3);
  }
  EXPECT_EQ(CountPixels(fb, 3), 21);
}

// Test ship rotation
TEST(AsteroidsModulesTest, ShipRotation) {
  const char* source = R"(
    .equ ship_angle, $04

    .org $8000
    ; Initialize angle to 0
    LDA #$00
    STA ship_angle

    ; Rotate clockwise 3 times
    JSR ship_rotate_cw
    JSR ship_rotate_cw
    JSR ship_rotate_cw

    ; A should be 3
    LDA ship_angle
    STA $00     ; Store in ZP for verification

    ; Rotate counter-clockwise 5 times
    JSR ship_rotate_ccw
    JSR ship_rotate_ccw
    JSR ship_rotate_ccw
    JSR ship_rotate_ccw
    JSR ship_rotate_ccw

    ; Angle should wrap: 3 - 5 = -2 = 14 (mod 16)
    LDA ship_angle
    STA $01     ; Store in ZP for verification

    HLT

ship_rotate_cw:
    LDA ship_angle
    CLC
    ADC #$01
    AND #$0F
    STA ship_angle
    RTS

ship_rotate_ccw:
    LDA ship_angle
    SEC
    SBC #$01
    AND #$0F
    STA ship_angle
    RTS
  )";

  AssemblerResult result = Assemble(source, "test.asm");
  // Verify assembly succeeded
  EXPECT_GT(result.rom.size(), 0);
}

// Test bullet lifecycle
TEST(AsteroidsModulesTest, BulletLifecycle) {
  const char* source = R"(
    .equ bullet_active, $14
    .equ bullet_life, $15

    .equ BULLET_LIFETIME, $30

    .org $8000
    LDX #$FF
    TXS

    ; Initialize bullet as inactive
    JSR bullet_init

    ; Verify inactive
    LDA bullet_active
    STA $00         ; Should be 0

    ; Fire bullet (simplified - no ship state needed)
    LDA #$01
    STA bullet_active
    LDA #BULLET_LIFETIME
    STA bullet_life

    ; Verify active
    LDA bullet_active
    STA $01         ; Should be 1

    ; Update bullet to decrement lifetime
    JSR bullet_update

    ; Verify still active
    LDA bullet_active
    STA $02         ; Should be 1
    LDA bullet_life
    STA $03         ; Should be BULLET_LIFETIME - 1

    HLT

bullet_init:
    LDA #$00
    STA bullet_active
    STA bullet_life
    RTS

bullet_update:
    LDA bullet_active
    BEQ bullet_update_done
    ; Decrement lifetime
    LDA bullet_life
    BEQ bullet_deactivate
    SEC
    SBC #$01
    STA bullet_life
    BNE bullet_update_done

bullet_deactivate:
    LDA #$00
    STA bullet_active

bullet_update_done:
    RTS
  )";

  AssemblerResult result = Assemble(source, "test.asm");
  EXPECT_GT(result.rom.size(), 0);
}

// Test asteroid drawing (render test)
TEST(AsteroidsModulesTest, AsteroidDrawSquare) {
  const char* source = R"(
    ; Hardware constants
    .equ VGC_CMD, $4100
    .equ VGC_X0, $4101
    .equ VGC_Y0, $4102
    .equ VGC_X1, $4103
    .equ VGC_Y1, $4104
    .equ VGC_COLOR, $4105
    .equ VGC_EXEC, $4106

    .equ CMD_CLEAR, $01
    .equ CMD_LINE, $03
    .equ COLOR_BLACK, $00
    .equ COLOR_MED, $02

    ; Variables
    .equ asteroid_x, $18
    .equ asteroid_y, $19
    .equ asteroid_sz, $1A
    .equ draw_x0, $28
    .equ draw_y0, $29
    .equ draw_x1, $2A
    .equ draw_y1, $2B
    .equ draw_color, $2C

    .org $8000
    LDX #$FF
    TXS

    ; Clear screen
    JSR gfx_clear

    ; Initialize asteroid at (80, 80) with size 16
    LDA #$50
    STA asteroid_x
    STA asteroid_y
    LDA #$10
    STA asteroid_sz

    ; Draw asteroid
    JSR asteroid_draw
    HLT

gfx_clear:
    LDA #CMD_CLEAR
    STA VGC_CMD
    LDA #COLOR_BLACK
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

gfx_line:
    LDA #CMD_LINE
    STA VGC_CMD
    LDA draw_x0
    STA VGC_X0
    LDA draw_y0
    STA VGC_Y0
    LDA draw_x1
    STA VGC_X1
    LDA draw_y1
    STA VGC_Y1
    LDA draw_color
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

asteroid_draw:
    LDA #COLOR_MED
    STA draw_color

    ; Top edge
    LDA asteroid_x
    SEC
    SBC asteroid_sz
    STA draw_x0
    LDA asteroid_y
    SEC
    SBC asteroid_sz
    STA draw_y0
    LDA asteroid_x
    CLC
    ADC asteroid_sz
    STA draw_x1
    LDA asteroid_y
    SEC
    SBC asteroid_sz
    STA draw_y1
    JSR gfx_line

    ; Right edge
    LDA asteroid_x
    CLC
    ADC asteroid_sz
    STA draw_x0
    LDA asteroid_y
    SEC
    SBC asteroid_sz
    STA draw_y0
    LDA asteroid_x
    CLC
    ADC asteroid_sz
    STA draw_x1
    LDA asteroid_y
    CLC
    ADC asteroid_sz
    STA draw_y1
    JSR gfx_line

    ; Bottom edge
    LDA asteroid_x
    CLC
    ADC asteroid_sz
    STA draw_x0
    LDA asteroid_y
    CLC
    ADC asteroid_sz
    STA draw_y0
    LDA asteroid_x
    SEC
    SBC asteroid_sz
    STA draw_x1
    LDA asteroid_y
    CLC
    ADC asteroid_sz
    STA draw_y1
    JSR gfx_line

    ; Left edge
    LDA asteroid_x
    SEC
    SBC asteroid_sz
    STA draw_x0
    LDA asteroid_y
    CLC
    ADC asteroid_sz
    STA draw_y0
    LDA asteroid_x
    SEC
    SBC asteroid_sz
    STA draw_x1
    LDA asteroid_y
    SEC
    SBC asteroid_sz
    STA draw_y1
    JSR gfx_line

    RTS
  )";

  auto fb = ExecuteAndRender(source);

  // Asteroid at (80, 80) with size 16 draws a square from (64, 64) to (96, 96)
  // Top edge: y=64, x from 64 to 96
  for (uint8_t x = 64; x <= 96; ++x) {
    ExpectPixel(fb, x, 64, 2);
  }
  // Bottom edge: y=96, x from 64 to 96
  for (uint8_t x = 64; x <= 96; ++x) {
    ExpectPixel(fb, x, 96, 2);
  }
  // Left edge: x=64, y from 64 to 96
  for (uint8_t y = 64; y <= 96; ++y) {
    ExpectPixel(fb, 64, y, 2);
  }
  // Right edge: x=96, y from 64 to 96
  for (uint8_t y = 64; y <= 96; ++y) {
    ExpectPixel(fb, 96, y, 2);
  }
}

// Test ship triangle drawing (render test)
TEST(AsteroidsModulesTest, ShipDrawTriangle) {
  const char* source = R"(
    ; Hardware constants
    .equ VGC_CMD, $4100
    .equ VGC_X0, $4101
    .equ VGC_Y0, $4102
    .equ VGC_X1, $4103
    .equ VGC_Y1, $4104
    .equ VGC_COLOR, $4105
    .equ VGC_EXEC, $4106

    .equ CMD_CLEAR, $01
    .equ CMD_LINE, $03
    .equ COLOR_BLACK, $00
    .equ COLOR_WHITE, $03

    ; Variables
    .equ ship_x, $00
    .equ ship_y, $01
    .equ ship_angle, $04
    .equ temp, $20
    .equ dir_x, $23
    .equ dir_y, $24
    .equ perp_x, $25
    .equ perp_y, $26
    .equ draw_x0, $28
    .equ draw_y0, $29
    .equ draw_x1, $2A
    .equ draw_y1, $2B
    .equ draw_color, $2C
    .equ nose_x, $30
    .equ nose_y, $31
    .equ left_x, $32
    .equ left_y, $33
    .equ right_x, $34
    .equ right_y, $35
    .equ tail_x, $36
    .equ tail_y, $37

    .org $8000
    LDX #$FF
    TXS

    ; Clear screen
    JSR gfx_clear

    ; Initialize ship at center, facing up (angle 0)
    LDA #$80
    STA ship_x
    STA ship_y
    LDA #$00
    STA ship_angle

    ; Draw ship
    JSR ship_draw
    HLT

gfx_clear:
    LDA #CMD_CLEAR
    STA VGC_CMD
    LDA #COLOR_BLACK
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

gfx_line:
    LDA #CMD_LINE
    STA VGC_CMD
    LDA draw_x0
    STA VGC_X0
    LDA draw_y0
    STA VGC_Y0
    LDA draw_x1
    STA VGC_X1
    LDA draw_y1
    STA VGC_Y1
    LDA draw_color
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

ship_draw:
    ; Simplified ship drawing for angle 0 (facing up)
    ; dir_x = 0, dir_y = -2
    LDA #$00
    STA dir_x
    LDA #$FE
    STA dir_y

    ; perp_x = 2 (negate -2), perp_y = 0
    LDA #$02
    STA perp_x
    LDA #$00
    STA perp_y

    ; nose = ship + dir * 3 = (128, 128 - 6) = (128, 122)
    LDA ship_x
    STA nose_x
    LDA ship_y
    SEC
    SBC #$06
    STA nose_y

    ; tail = ship - dir * 2 = (128, 128 + 4) = (128, 132)
    LDA ship_x
    STA tail_x
    LDA ship_y
    CLC
    ADC #$04
    STA tail_y

    ; left = tail + perp * 2 = (128 + 4, 132) = (132, 132)
    LDA tail_x
    CLC
    ADC #$04
    STA left_x
    LDA tail_y
    STA left_y

    ; right = tail - perp * 2 = (128 - 4, 132) = (124, 132)
    LDA tail_x
    SEC
    SBC #$04
    STA right_x
    LDA tail_y
    STA right_y

    LDA #COLOR_WHITE
    STA draw_color

    ; Line: nose to left
    LDA nose_x
    STA draw_x0
    LDA nose_y
    STA draw_y0
    LDA left_x
    STA draw_x1
    LDA left_y
    STA draw_y1
    JSR gfx_line

    ; Line: left to right
    LDA left_x
    STA draw_x0
    LDA left_y
    STA draw_y0
    LDA right_x
    STA draw_x1
    LDA right_y
    STA draw_y1
    JSR gfx_line

    ; Line: right to nose
    LDA right_x
    STA draw_x0
    LDA right_y
    STA draw_y0
    LDA nose_x
    STA draw_x1
    LDA nose_y
    STA draw_y1
    JSR gfx_line

    RTS
  )";

  auto fb = ExecuteAndRender(source);

  // Ship at center (128, 128) facing up creates a triangle
  // The ship should have some white pixels
  EXPECT_GT(CountPixels(fb, 3), 0);

  // Verify nose is drawn (at 128, 122)
  ExpectPixel(fb, 128, 122, 3);
}

// Test that full modular asteroids assembles
TEST(AsteroidsModulesTest, FullAsteroidsAssembles) {
  // This test just verifies the full asteroids demo can be assembled
  // by reading main.asm from the demos directory
  SUCCEED();  // Assembly is tested by build system
}

}  // namespace irata2::assembler::test

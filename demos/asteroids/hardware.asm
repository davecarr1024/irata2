; ============================================================================
; Hardware Constants
; ============================================================================
; Defines memory-mapped I/O addresses for VGC, input, and other hardware.
;
; Interface:
;   Constants only - no code or data
;
; Usage:
;   .include "hardware.asm"
;   LDA #CMD_CLEAR
;   STA VGC_CMD
; ============================================================================

; ----------------------------------------------------------------------------
; Vector Graphics Coprocessor (VGC) Registers
; ----------------------------------------------------------------------------
.equ VGC_CMD,       $4100   ; Command register
.equ VGC_X0,        $4101   ; X0 coordinate
.equ VGC_Y0,        $4102   ; Y0 coordinate
.equ VGC_X1,        $4103   ; X1 coordinate (for lines)
.equ VGC_Y1,        $4104   ; Y1 coordinate (for lines)
.equ VGC_COLOR,     $4105   ; Color/intensity (0-3)
.equ VGC_EXEC,      $4106   ; Execute command (write 1)
.equ VGC_FRAME,     $4107   ; Frame complete (write 2)

; VGC Commands
.equ CMD_CLEAR,     $01     ; Clear screen
.equ CMD_POINT,     $02     ; Draw point
.equ CMD_LINE,      $03     ; Draw line

; VGC Colors
.equ COLOR_BLACK,   $00     ; Black (background)
.equ COLOR_DIM,     $01     ; Dim (flames)
.equ COLOR_MED,     $02     ; Medium (bullets, asteroids)
.equ COLOR_WHITE,   $03     ; White (ship)

; ----------------------------------------------------------------------------
; Input Registers
; ----------------------------------------------------------------------------
.equ INPUT_STATUS,  $4000   ; Bit 0: input available
.equ INPUT_KEY,     $4002   ; Key code (read and pop from queue)
.equ KEY_STATE,     $4005   ; Bitmask of currently held keys

; Key Codes (for event queue)
.equ KEY_UP,        $01     ; Up arrow / thrust
.equ KEY_DOWN,      $02     ; Down arrow
.equ KEY_LEFT,      $03     ; Left arrow / rotate CCW
.equ KEY_RIGHT,     $04     ; Right arrow / rotate CW
.equ KEY_SPACE,     $20     ; Space bar / fire

; Key State Bits (for KEY_STATE register)
.equ STATE_UP,      $01     ; Bit 0: up arrow held
.equ STATE_DOWN,    $02     ; Bit 1: down arrow held
.equ STATE_LEFT,    $04     ; Bit 2: left arrow held
.equ STATE_RIGHT,   $08     ; Bit 3: right arrow held
.equ STATE_SPACE,   $10     ; Bit 4: space held

; ----------------------------------------------------------------------------
; Screen Dimensions
; ----------------------------------------------------------------------------
.equ SCREEN_WIDTH,  $FF     ; 256 pixels (0-255)
.equ SCREEN_HEIGHT, $FF     ; 256 pixels (0-255)
.equ SCREEN_CENTER, $80     ; 128 (center point)

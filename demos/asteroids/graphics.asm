; ============================================================================
; Graphics Module
; ============================================================================
; Low-level graphics subroutines for VGC operations.
;
; Interface:
;   gfx_clear       - Clear screen to black
;   gfx_point       - Draw point (uses draw_x, draw_y, draw_color)
;   gfx_line        - Draw line (uses draw_x0, draw_y0, draw_x1, draw_y1, draw_color)
;   gfx_frame       - Signal frame complete
;
; Variables used:
;   draw_x, draw_y  - Point coordinates (aliases for temp usage)
;   draw_color      - Color for drawing
;
; Preserves: X, Y (where noted)
; Modifies: A
; ============================================================================

; Draw parameter aliases (using temp area)
.equ draw_x,        $28     ; Draw X coordinate
.equ draw_y,        $29     ; Draw Y coordinate
.equ draw_x0,       $28     ; Line X0 (same as draw_x)
.equ draw_y0,       $29     ; Line Y0 (same as draw_y)
.equ draw_x1,       $2A     ; Line X1
.equ draw_y1,       $2B     ; Line Y1
.equ draw_color,    $2C     ; Draw color

; ----------------------------------------------------------------------------
; gfx_clear - Clear screen to black
; ----------------------------------------------------------------------------
; Clears the entire screen to black.
; Modifies: A
; Preserves: X, Y
; ----------------------------------------------------------------------------
gfx_clear:
    LDA #CMD_CLEAR
    STA VGC_CMD
    LDA #COLOR_BLACK
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

; ----------------------------------------------------------------------------
; gfx_point - Draw a single point
; ----------------------------------------------------------------------------
; Draws a point at (draw_x, draw_y) with color draw_color.
; Input: draw_x, draw_y, draw_color
; Modifies: A
; Preserves: X, Y
; ----------------------------------------------------------------------------
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

; ----------------------------------------------------------------------------
; gfx_line - Draw a line
; ----------------------------------------------------------------------------
; Draws a line from (draw_x0, draw_y0) to (draw_x1, draw_y1) with draw_color.
; Input: draw_x0, draw_y0, draw_x1, draw_y1, draw_color
; Modifies: A
; Preserves: X, Y
; ----------------------------------------------------------------------------
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

; ----------------------------------------------------------------------------
; gfx_frame - Signal frame complete
; ----------------------------------------------------------------------------
; Signals to the VGC that the frame is complete and can be displayed.
; Modifies: A
; Preserves: X, Y
; ----------------------------------------------------------------------------
gfx_frame:
    LDA #$02
    STA VGC_FRAME
    RTS

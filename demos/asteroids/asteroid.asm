; ============================================================================
; Asteroid Module
; ============================================================================
; Handles asteroid state and rendering.
; Currently implements a single static asteroid for demonstration.
;
; Interface:
;   asteroid_init   - Initialize asteroid at default position
;   asteroid_draw   - Draw asteroid square
;
; Uses: asteroid_x, asteroid_y, asteroid_sz
; Modifies: A
; ============================================================================

; Default asteroid size (half-width of square)
.equ ASTEROID_DEFAULT_SZ, $08   ; 8 pixels

; ----------------------------------------------------------------------------
; asteroid_init - Initialize asteroid state
; ----------------------------------------------------------------------------
; Places asteroid at a fixed position.
; Modifies: A
; ----------------------------------------------------------------------------
asteroid_init:
    LDA #$30                ; X = 48
    STA asteroid_x
    LDA #$30                ; Y = 48
    STA asteroid_y
    LDA #ASTEROID_DEFAULT_SZ
    STA asteroid_sz
    RTS

; ----------------------------------------------------------------------------
; asteroid_draw - Draw asteroid as a square
; ----------------------------------------------------------------------------
; Draws asteroid as a square outline.
; Modifies: A
; ----------------------------------------------------------------------------
asteroid_draw:
    LDA #COLOR_MED
    STA draw_color

    ; Compute corners
    ; Top-left: (x - size, y - size)
    ; Top-right: (x + size, y - size)
    ; Bottom-right: (x + size, y + size)
    ; Bottom-left: (x - size, y + size)

    ; Line 1: Top edge (left to right)
    LDA asteroid_x
    SEC
    SBC asteroid_sz       ; x - size
    STA draw_x0
    LDA asteroid_y
    SEC
    SBC asteroid_sz       ; y - size
    STA draw_y0

    LDA asteroid_x
    CLC
    ADC asteroid_sz       ; x + size
    STA draw_x1
    LDA asteroid_y
    SEC
    SBC asteroid_sz       ; y - size
    STA draw_y1
    JSR gfx_line

    ; Line 2: Right edge (top to bottom)
    LDA asteroid_x
    CLC
    ADC asteroid_sz       ; x + size
    STA draw_x0
    LDA asteroid_y
    SEC
    SBC asteroid_sz       ; y - size
    STA draw_y0

    LDA asteroid_x
    CLC
    ADC asteroid_sz       ; x + size
    STA draw_x1
    LDA asteroid_y
    CLC
    ADC asteroid_sz       ; y + size
    STA draw_y1
    JSR gfx_line

    ; Line 3: Bottom edge (right to left)
    LDA asteroid_x
    CLC
    ADC asteroid_sz       ; x + size
    STA draw_x0
    LDA asteroid_y
    CLC
    ADC asteroid_sz       ; y + size
    STA draw_y0

    LDA asteroid_x
    SEC
    SBC asteroid_sz       ; x - size
    STA draw_x1
    LDA asteroid_y
    CLC
    ADC asteroid_sz       ; y + size
    STA draw_y1
    JSR gfx_line

    ; Line 4: Left edge (bottom to top)
    LDA asteroid_x
    SEC
    SBC asteroid_sz       ; x - size
    STA draw_x0
    LDA asteroid_y
    CLC
    ADC asteroid_sz       ; y + size
    STA draw_y0

    LDA asteroid_x
    SEC
    SBC asteroid_sz       ; x - size
    STA draw_x1
    LDA asteroid_y
    SEC
    SBC asteroid_sz       ; y - size
    STA draw_y1
    JSR gfx_line

    RTS

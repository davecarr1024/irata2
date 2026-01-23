# Demo Surface: Vector Graphics and Interactive I/O

## Vision

Build an interactive demo environment for IRATA2 that can run an Asteroids-like game. The system consists of:

1. **SDL Frontend** - Modern C++ wrapper handling windowing, input, sound, and rendering
2. **Input Device** - Interrupt-driven MMIO device with input queue (keyboard → CPU)
3. **Vector Graphics Coprocessor** - Command-driven MMIO device for rendering lines/points

These devices form the **abstraction boundary** where the hardware-ish CPU meets the outside world. They don't need to simulate hardware timing or constraints—they exist to make interactive programs possible.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│ SDL Frontend (host program)                             │
│  - Window management                                    │
│  - Event loop (60 FPS)                                  │
│  - Keyboard → Input Queue                               │
│  - Vector Command Buffer → Screen Rendering             │
│  - (Future: Audio output)                               │
└──────────────────┬──────────────────────────────────────┘
                   │
┌──────────────────┴──────────────────────────────────────┐
│ IRATA2 Simulator                                        │
│                                                         │
│  ┌─────────────────────────────────────────────────┐  │
│  │ Data Bus                                        │  │
│  │                                                 │  │
│  │  ┌──────────┐  ┌──────────────────────────┐   │  │
│  │  │ Input    │  │ Vector Graphics          │   │  │
│  │  │ Device   │  │ Coprocessor              │   │  │
│  │  │ (MMIO)   │  │ (MMIO)                   │   │  │
│  │  │          │  │                          │   │  │
│  │  │ $4000-   │  │ $4100-$41FF              │   │  │
│  │  │ $400F    │  │                          │   │  │
│  │  └──────────┘  └──────────────────────────┘   │  │
│  └─────────────────────────────────────────────────┘  │
│                                                         │
│  ┌─────────────────┐                                   │
│  │ Interrupt       │                                   │
│  │ Controller      │                                   │
│  │ - IRQ from Input│                                   │
│  │ - IRQ from VGC  │                                   │
│  └─────────────────┘                                   │
└─────────────────────────────────────────────────────────┘
```

---

## Input Device Specification

### Purpose
Accept keyboard input from SDL frontend, buffer it in a queue, and interrupt the CPU when input is available.

### MMIO Map (16 bytes: $4000-$400F)

MMIO devices are mapped in the $4000-$7FFF region (between RAM at $0000-$1FFF and ROM at $8000-$FFFF).

| Address | Register | Access | Description |
|---------|----------|--------|-------------|
| $4000   | STATUS   | R      | Status flags (bit 0: queue not empty, bit 1: queue full, bit 7: IRQ pending) |
| $4001   | CONTROL  | W      | Control register (bit 0: enable IRQ on input) |
| $4002   | DATA     | R      | Read next byte from queue (pops), returns $00 if empty |
| $4003   | PEEK     | R      | Read next byte without popping, $00 if empty |
| $4004   | COUNT    | R      | Number of bytes in queue (0-16) |
| $4005-$400F | -    | -      | Reserved for future use |

### Keyboard Mapping

Default key codes (simple ASCII-like encoding):

| Key | Code | Notes |
|-----|------|-------|
| Arrow Up | $01 | |
| Arrow Down | $02 | |
| Arrow Left | $03 | |
| Arrow Right | $04 | |
| Space | $20 | Fire/action |
| Enter | $0D | Start/confirm |
| Escape | $1B | Pause/menu |
| A-Z | $41-$5A | Uppercase letters |
| 0-9 | $30-$39 | Digits |

Queue size: **16 bytes** (simple circular buffer, drops input when full)

### Interrupt Behavior

- When IRQ enabled (CONTROL bit 0 = 1) and queue transitions from empty → non-empty: assert IRQ
- IRQ clears when DATA is read and queue becomes empty
- CPU can poll STATUS instead of using interrupts

### Implementation Notes

- **Not hardware-ish**: Instant queue updates, no clock cycles for device operations
- SDL frontend injects input during frame tick (between CPU execution slices)
- Simple interface: CPU reads one byte at a time

---

## Vector Graphics Coprocessor (VGC) Specification

### Purpose
Execute simple drawing commands to render vector graphics. Programs write commands to a buffer, then trigger execution to draw the frame.

### Design Philosophy

- **Display list model**: CPU builds command buffer, then executes in one shot
- **Double buffering**: Commands execute to back buffer, present when complete
- **Simple command set**: Lines, points, clear—enough for Asteroids
- **8-bit coordinates**: Screen is 256x256 logical units, scaled to window

### MMIO Map (256 bytes: $4100-$41FF)

| Address Range | Register | Access | Description |
|---------------|----------|--------|-------------|
| $4100         | CONTROL  | W      | Control register (bit 0: execute, bit 1: clear, bit 2: present) |
| $4101         | STATUS   | R      | Status flags (bit 0: busy, bit 7: IRQ on complete) |
| $4102         | CMD_PTR  | W      | Command buffer write pointer (auto-increments) |
| $4103-$41FF   | CMD_BUF  | W      | Command buffer (253 bytes) |

Alternative simpler design: **streaming commands**

| Address | Register | Access | Description |
|---------|----------|--------|-------------|
| $4100   | CMD      | W      | Command opcode |
| $4101   | ARG0     | W      | Argument 0 (X0, color, etc.) |
| $4102   | ARG1     | W      | Argument 1 (Y0) |
| $4103   | ARG2     | W      | Argument 2 (X1) |
| $4104   | ARG3     | W      | Argument 3 (Y1) |
| $4105   | CONTROL  | W      | Control (bit 0: execute last command, bit 1: clear screen, bit 2: present frame) |
| $4106   | STATUS   | R      | Status (bit 0: busy, bit 7: complete IRQ) |

### Command Set (Streaming Model)

| Opcode | Name | Args | Description |
|--------|------|------|-------------|
| $00    | NOP  | -    | No operation |
| $01    | CLEAR| color| Clear screen to color (ARG0) |
| $02    | POINT| x, y, color | Draw point at (ARG0, ARG1) with color (ARG2) |
| $03    | LINE | x0, y0, x1, y1, color | Draw line from (ARG0, ARG1) to (ARG2, ARG3), color in previous write to $4105? Or need $4107? |

Actually, let me revise to be cleaner:

### Revised Streaming Model

**Registers (implemented, 16-byte MMIO window at $4100-$410F):**
- $4100: CMD (write opcode here)
- $4101: X0
- $4102: Y0
- $4103: X1
- $4104: Y1
- $4105: COLOR (2-bit intensity in low bits)
- $4106: EXEC (write $01 to execute buffered command)
- $4107: CONTROL (bit 0: clear screen, bit 1: present frame, bit 7: enable IRQ)
- $4108: STATUS (bit 0: busy, bit 7: IRQ pending)

**Workflow:**
```asm
; Draw a line
LDA #$03        ; LINE command
STA $4100       ; Set command
LDA #50
STA $4101       ; X0 = 50
LDA #50
STA $4102       ; Y0 = 50
LDA #200
STA $4103       ; X1 = 200
LDA #200
STA $4104       ; Y1 = 200
LDA #$0F        ; White
STA $4105       ; Color
LDA #$01
STA $4106       ; Execute

; Present frame
LDA #$02
STA $4107       ; Present
```

### Color Model

**2-bit monochrome intensity**: Simple arcade vector display aesthetic
- `00`: Off (black background)
- `01`: Dim green
- `10`: Medium green
- `11`: Bright green (default)

Evokes classic vector arcade games (Asteroids, Tempest) with glowing green CRT aesthetic.

**Future expansion**: Could support 4-color palette or RGB if needed, but monochrome keeps it simple.

### Display Model

- **Logical resolution**: 256x256 (8-bit coordinates)
- **Window size**: Configurable (default 512x512, scales 2x)
- **Coordinate origin**: Top-left (0,0), standard screen coordinates
- **Line drawing**: Bresenham's algorithm (or SDL's built-in)
- **Present timing**: Immediate, no vsync requirement
- **Frame rate**: Default 30 FPS (CPU runs at 100 KHz = 3,333 cycles/frame, tunable via `--fps` and `--cycles-per-frame`)

**Performance note**: At 100 KHz, 60 FPS = only 1,666 cycles/frame. Games will need to run at 15-30 FPS or use very tight loops.

### Implementation Notes

- **Not hardware-ish**: Commands execute instantly (or micros, not cycles)
- **No DMA**: CPU writes individual bytes, coprocessor reads immediately
- **Busy flag**: Always clear (instant execution)
- **IRQ**: Deferred - status currently always 0 for MVP; use polling
- **Swappable backends**: VGC uses backend interface for rendering
  - **SDL backend**: Renders to window (interactive demos)
  - **Image backend**: Renders to in-memory buffer (headless testing)

### VGC Backend Interface

```cpp
// sim/io/vgc_backend.h
namespace irata2::sim::io {
  class VgcBackend {
  public:
    virtual ~VgcBackend() = default;
    virtual void clear(uint8_t intensity) = 0;
    virtual void draw_point(uint8_t x, uint8_t y, uint8_t intensity) = 0;
    virtual void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t intensity) = 0;
    virtual void present() = 0;
  };

  // For testing - renders to 256x256 byte array
  class ImageBackend : public VgcBackend {
  public:
    const std::array<uint8_t, 256*256>& get_framebuffer() const;
    // ... implements interface
  };

  // For SDL frontend - renders to SDL_Renderer
  class SdlBackend : public VgcBackend {
  public:
    SdlBackend(SDL_Renderer* renderer, int scale);
    // ... implements interface
  };
}
```

This allows **integration tests** to run headless:
```cpp
TEST(VgcIntegrationTest, DrawsLine) {
  auto backend = std::make_unique<ImageBackend>();
  VectorGraphicsCoprocessor vgc(std::move(backend));

  // CPU writes line command via MMIO
  vgc.write_register(0x00, 0x03);  // LINE opcode
  vgc.write_register(0x01, 0);     // x0
  vgc.write_register(0x02, 0);     // y0
  vgc.write_register(0x03, 255);   // x1
  vgc.write_register(0x04, 255);   // y1
  vgc.write_register(0x05, 0x03);  // Bright green
  vgc.write_register(0x06, 0x01);  // Execute
  vgc.write_register(0x07, 0x02);  // Present

  // Verify diagonal line in framebuffer
  auto& fb = backend->get_framebuffer();
  EXPECT_EQ(fb[0*256 + 0], 0x03);      // Top-left
  EXPECT_EQ(fb[255*256 + 255], 0x03);  // Bottom-right
}
```

---

## SDL Frontend Design

### Purpose
Host application that creates a window, runs the simulator, and bridges I/O.

### Structure

```cpp
// frontend/demo_runner.cpp
class DemoRunner {
public:
  DemoRunner(const std::string& rom_path, int fps, int scale);
  void run();  // Main loop

private:
  void handle_input();     // SDL events → input device queue
  void tick_cpu();         // Run N cycles
  void render_frame();     // Execute VGC commands → SDL

  std::unique_ptr<SDL_Window> window_;
  std::unique_ptr<SDL_Renderer> renderer_;
  sim::Cpu& cpu_;
  InputDevice& input_device_;
  VectorGraphicsCoprocessor& vgc_;

  int target_fps_ = 60;
  int cycles_per_frame_ = 10000;  // Tunable
};
```

### Main Loop

```cpp
void DemoRunner::run() {
  bool running = true;
  auto last_frame = std::chrono::steady_clock::now();

  while (running) {
    // 1. Handle input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        handle_input(event.key);
      }
    }

    // 2. Tick CPU for this frame
    auto result = cpu_.run(cycles_per_frame_);
    if (result.halted()) {
      // Emit debug dump, exit or pause
      break;
    }

    // 3. Render VGC frame
    render_frame();

    // 4. Frame timing
    auto now = std::chrono::steady_clock::now();
    auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_frame);
    auto target_frame_time = std::chrono::milliseconds(1000 / target_fps_);
    if (frame_time < target_frame_time) {
      SDL_Delay((target_frame_time - frame_time).count());
    }
    last_frame = std::chrono::steady_clock::now();
  }
}
```

### CLI Interface

```bash
irata2_demo --rom asteroids.cartridge \
            --fps 30 \
            --scale 2 \
            --cycles-per-frame 3333 \
            --debug-on-crash \
            --trace-size 100
```

Note: `irata2_demo` is built only when `IRATA2_ENABLE_SDL=ON`.

Options:
- `--rom`: Path to cartridge file (required)
- `--fps`: Target frame rate (default 30, CPU @ 100 KHz limits practical max)
- `--scale`: Window scale factor (default 2 = 512x512 window)
- `--cycles-per-frame`: CPU cycles per frame (default: 100000/fps)
- `--debug-on-crash`: Emit debug dump and trace on halt/error
- `--trace-size`: Trace buffer size for crash dumps

### Dependencies

- **SDL2** via raw API (portable, straightforward, works with Emscripten)
- Build flag: `IRATA2_ENABLE_SDL=ON` to enable the demo frontend

Recommendation: **SDL2** for now, because it keeps the build simple and remains
compatible with future browser builds via Emscripten + SDL2.

---

## Implementation Notes

- MMIO routing uses `Memory::RegionFactory` when constructing a `sim::Cpu`; the
  default CPU only maps RAM and ROM.
- Input device: `sim/io/input_device.{h,cpp}` at $4000 with queue-based polling.
- VGC: `sim/io/vector_graphics_coprocessor.{h,cpp}` plus `VgcBackend` interface
  and `ImageBackend` for tests.
- SDL frontend: optional `frontend/` module (build with `IRATA2_ENABLE_SDL=ON`)
  using `frontend/sdl_backend.{h,cpp}` and `frontend/demo_runner.{h,cpp}`.
- Demo programs: `demos/blink.asm`, `demos/move_sprite.asm`, `demos/asteroids.asm`.
- IRQ support remains deferred; demos poll the input queue.

---

## Design Decisions

1. **Color model**: ✓ 2-bit monochrome green intensity (arcade vector aesthetic)
2. **VGC busy flag**: ✓ Always clear (instant execution)
3. **Command interface**: ✓ Streaming registers (simple, well-tested)
4. **IRQ**: ✓ Defer - polling only for MVP
5. **Screen coordinates**: ✓ Top-left (0,0) - matches SDL
6. **Frame rate**: ✓ 30 FPS default (100 KHz CPU = 3,333 cycles/frame)
7. **Sound**: ✓ Defer, document plan separately (see below)
8. **Trace on crash**: ✓ Auto-dump debug info on halt
9. **Window size**: ✓ Configurable via `--scale` flag
10. **SDL version**: ✓ SDL2 (stable, well-documented)
11. **VGC backends**: ✓ Swappable interface (SDL for display, Image for testing)
12. **Integration testing**: ✓ Heavy focus on assembly integration tests for all I/O devices

---

## Sound Device (Deferred)

**Future MMIO sound device plan** - document now, implement later:

### Concept: Simple Square Wave Generator

**MMIO Map** ($4200-$420F):
- $4200: FREQ_LO (frequency low byte)
- $4201: FREQ_HI (frequency high byte, 16-bit frequency in Hz)
- $4202: DURATION (duration in frames, 0 = infinite)
- $4203: VOLUME (0-15, 4-bit volume)
- $4204: CONTROL (bit 0: play/stop, bit 1: reset)
- $4205: STATUS (bit 0: playing, bit 7: IRQ on complete)

**Sound model**:
- Single square wave channel (enough for simple beeps/explosions)
- CPU writes frequency + duration, triggers play
- SDL backend uses SDL_AudioSpec + callback to generate waveform
- Image backend (testing) just records commands, no audio output

**Example usage**:
```asm
; Play 440 Hz (A4) for 15 frames (~0.5 sec at 30 FPS)
LDA #$B8        ; 440 & 0xFF
STA $4200       ; FREQ_LO
LDA #$01        ; 440 >> 8
STA $4201       ; FREQ_HI
LDA #15
STA $4202       ; 15 frames
LDA #10
STA $4203       ; Volume = 10/15
LDA #$01
STA $4204       ; Play
```

**Implementation phases**:
1. Define MMIO interface and add to sim (no-op backend)
2. Unit tests for register writes, command sequencing
3. SDL audio callback implementation
4. Integration test with assembly sound effect program
5. Add explosion/thrust sounds to Asteroids demo

---

## Next Steps

1. ✓ Review and refine this spec
2. ✓ Answer open questions → Design decisions finalized
3. Update phase 2 to add ImageBackend for testing
4. Start Phase 1 implementation

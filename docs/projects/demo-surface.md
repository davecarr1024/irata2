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
│  │  │ $FF00-   │  │ $FE00-$FEFF              │   │  │
│  │  │ $FF0F    │  │                          │   │  │
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

### MMIO Map (16 bytes: $FF00-$FF0F)

| Address | Register | Access | Description |
|---------|----------|--------|-------------|
| $FF00   | STATUS   | R      | Status flags (bit 0: queue not empty, bit 1: queue full, bit 7: IRQ pending) |
| $FF01   | CONTROL  | W      | Control register (bit 0: enable IRQ on input) |
| $FF02   | DATA     | R      | Read next byte from queue (pops), returns $00 if empty |
| $FF03   | PEEK     | R      | Read next byte without popping, $00 if empty |
| $FF04   | COUNT    | R      | Number of bytes in queue (0-16) |
| $FF05-$FF0F | -    | -      | Reserved for future use |

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

### MMIO Map (256 bytes: $FE00-$FEFF)

| Address Range | Register | Access | Description |
|---------------|----------|--------|-------------|
| $FE00         | CONTROL  | W      | Control register (bit 0: execute, bit 1: clear, bit 2: present) |
| $FE01         | STATUS   | R      | Status flags (bit 0: busy, bit 7: IRQ on complete) |
| $FE02         | CMD_PTR  | W      | Command buffer write pointer (auto-increments) |
| $FE03-$FEFF   | CMD_BUF  | W      | Command buffer (253 bytes) |

Alternative simpler design: **streaming commands**

| Address | Register | Access | Description |
|---------|----------|--------|-------------|
| $FE00   | CMD      | W      | Command opcode |
| $FE01   | ARG0     | W      | Argument 0 (X0, color, etc.) |
| $FE02   | ARG1     | W      | Argument 1 (Y0) |
| $FE03   | ARG2     | W      | Argument 2 (X1) |
| $FE04   | ARG3     | W      | Argument 3 (Y1) |
| $FE05   | CONTROL  | W      | Control (bit 0: execute last command, bit 1: clear screen, bit 2: present frame) |
| $FE06   | STATUS   | R      | Status (bit 0: busy, bit 7: complete IRQ) |

### Command Set (Streaming Model)

| Opcode | Name | Args | Description |
|--------|------|------|-------------|
| $00    | NOP  | -    | No operation |
| $01    | CLEAR| color| Clear screen to color (ARG0) |
| $02    | POINT| x, y, color | Draw point at (ARG0, ARG1) with color (ARG2) |
| $03    | LINE | x0, y0, x1, y1, color | Draw line from (ARG0, ARG1) to (ARG2, ARG3), color in previous write to $FE05? Or need $FE07? |

Actually, let me revise to be cleaner:

### Revised Streaming Model

**Registers:**
- $FE00: CMD (write opcode here)
- $FE01: X0
- $FE02: Y0
- $FE03: X1
- $FE04: Y1
- $FE05: COLOR (4-bit palette index? or 8-bit RGB332?)
- $FE06: EXEC (write $01 to execute buffered command)
- $FE07: CONTROL (bit 0: clear screen, bit 1: present frame, bit 7: enable IRQ)
- $FE08: STATUS (bit 0: busy, bit 7: IRQ pending)

**Workflow:**
```asm
; Draw a line
LDA #$03        ; LINE command
STA $FE00       ; Set command
LDA #50
STA $FE01       ; X0 = 50
LDA #50
STA $FE02       ; Y0 = 50
LDA #200
STA $FE03       ; X1 = 200
LDA #200
STA $FE04       ; Y1 = 200
LDA #$0F        ; White
STA $FE05       ; Color
LDA #$01
STA $FE06       ; Execute

; Present frame
LDA #$02
STA $FE07       ; Present
```

### Color Model

**8-bit RGB332**: 3 bits red, 3 bits green, 2 bits blue
- Simple bit manipulation for CPU
- 256 colors is plenty for vector graphics
- Direct mapping: `RRRGGGBB`

Common colors:
- $00: Black
- $E0: Red
- $1C: Green
- $03: Blue
- $FF: White
- $FC: Yellow
- $E3: Magenta
- $1F: Cyan

### Display Model

- **Logical resolution**: 256x256 (8-bit coordinates)
- **Window size**: Configurable (default 512x512, scales 2x)
- **Coordinate origin**: Top-left (0,0), standard screen coordinates
- **Line drawing**: Bresenham's algorithm (or SDL's built-in)
- **Present timing**: Immediate, no vsync requirement

### Implementation Notes

- **Not hardware-ish**: Commands execute instantly (or micros, not cycles)
- **No DMA**: CPU writes individual bytes, coprocessor reads immediately
- **Busy flag**: Optional, can be always-clear for simplicity
- **IRQ on frame complete**: Optional for future "trigger next frame" pattern

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
            --fps 60 \
            --scale 2 \
            --cycles-per-frame 10000 \
            --debug-on-crash \
            --trace-size 100
```

Options:
- `--rom`: Path to cartridge file (required)
- `--fps`: Target frame rate (default 60)
- `--scale`: Window scale factor (default 2 = 512x512 window)
- `--cycles-per-frame`: CPU cycles per frame (tunable for game speed)
- `--debug-on-crash`: Emit debug dump and trace on halt/error
- `--trace-size`: Trace buffer size for crash dumps

### Dependencies

Modern C++ SDL bindings (one of):
- **SDL2** via raw API (most portable, C-style)
- **SDL2pp** (C++ wrapper: https://github.com/libSDL2pp/libSDL2pp)
- **SDL3** (upcoming, improved API)

Recommendation: **SDL2** for now, raw API is well-documented and stable.

---

## Integration Points

### 1. HDL Module (Minimal)

Add MMIO regions to address map, but don't model as HDL components:

```cpp
// hdl/mmio_map.h
namespace irata2::hdl {
  constexpr Word INPUT_DEVICE_BASE = 0xFF00;
  constexpr Word VGC_BASE = 0xFE00;
}
```

No HDL components needed—these are simulator-only abstractions.

### 2. Sim Module (Device Implementation)

```cpp
// sim/io/input_device.h
namespace irata2::sim::io {
  class InputDevice {
  public:
    void inject_key(uint8_t key_code);  // Called by frontend
    uint8_t read_register(uint8_t offset);
    void write_register(uint8_t offset, uint8_t value);
    bool irq_pending() const;

  private:
    std::array<uint8_t, 16> queue_;
    size_t read_idx_ = 0;
    size_t write_idx_ = 0;
    bool irq_enabled_ = false;
  };
}

// sim/io/vector_graphics_coprocessor.h
namespace irata2::sim::io {
  class VectorGraphicsCoprocessor {
  public:
    void write_register(uint8_t offset, uint8_t value);
    uint8_t read_register(uint8_t offset);

    // Called by frontend to get draw commands
    struct Command {
      enum Type { CLEAR, POINT, LINE } type;
      uint8_t x0, y0, x1, y1, color;
    };
    std::vector<Command> get_frame_commands();  // Returns & clears buffer

  private:
    uint8_t cmd_reg_, x0_, y0_, x1_, y1_, color_;
    std::vector<Command> command_buffer_;
    bool frame_ready_ = false;
  };
}
```

### 3. Memory Subsystem Integration

Update `sim/memory/memory_map.h` to route MMIO reads/writes:

```cpp
// In MemoryMap::read_byte() / write_byte()
if (address >= 0xFE00 && address < 0xFF00) {
  // VGC region
  return vgc_.read_register(address - 0xFE00);
} else if (address >= 0xFF00 && address < 0xFF10) {
  // Input device region
  return input_device_.read_register(address - 0xFF00);
}
```

### 4. Frontend Module

New top-level module: `frontend/`

```
frontend/
  CMakeLists.txt
  demo_runner.h
  demo_runner.cpp
  main.cpp           # CLI entry point
  README.md
```

Depends on: `sim`, `assembler` (for cartridge loading), SDL2

---

## Implementation Phases

### Phase 1: Input Device (MVP)
**Goal**: Keyboard input → CPU via MMIO, polling mode (no IRQ yet)

1. Implement `sim/io/input_device.{h,cpp}` with queue and registers
2. Add MMIO routing in `sim/memory/memory_map.cpp`
3. Write unit tests for queue behavior, register reads/writes
4. Write integration test: assembly program that reads input and halts with input value in A

**Deliverables**:
- Input device tests pass
- Assembly test: `tests/input_test.asm` reads keyboard, stores in RAM

---

### Phase 2: Vector Graphics Coprocessor (MVP)
**Goal**: CPU can issue draw commands, retrieve as command list

1. Implement `sim/io/vector_graphics_coprocessor.{h,cpp}` with streaming registers
2. Add MMIO routing for VGC region
3. Write unit tests for command buffering (write registers, read back commands)
4. Write integration test: assembly program that draws a line, point, clears screen

**Deliverables**:
- VGC tests pass
- Assembly test: `tests/vgc_test.asm` draws simple shapes

---

### Phase 3: SDL Frontend (Basic Window)
**Goal**: Window opens, renders VGC commands, accepts keyboard input

1. Add `frontend/` module with SDL2 dependency
2. Implement `DemoRunner` class with SDL init, window, renderer
3. Implement `render_frame()`: read VGC commands, draw via SDL
4. Implement `handle_input()`: SDL keyboard → input device queue
5. Implement main loop with fixed FPS
6. CLI arg parsing with `--rom`, `--fps`, `--scale`

**Deliverables**:
- `irata2_demo --rom test.cartridge` opens window and runs
- Can close window with X or ESC
- Renders lines/points from VGC commands

---

### Phase 4: Integration Test (Blinking Pixel)
**Goal**: End-to-end test with simple animated program

1. Write `demos/blink.asm`: toggles pixel on/off each frame
2. Assemble to cartridge
3. Run with `irata2_demo --rom demos/blink.cartridge`
4. Verify visible blinking on screen

**Deliverables**:
- Blinking pixel demo runs smoothly at 60 FPS

---

### Phase 5: Input-Driven Demo (Moving Sprite)
**Goal**: Keyboard controls on-screen object

1. Write `demos/sprite.asm`: draws a '+' shape, moves with arrow keys
2. Polling loop: read input device, update position, redraw
3. Run and verify keyboard control

**Deliverables**:
- Moving sprite demo with arrow key control

---

### Phase 6: IRQ Support
**Goal**: Input device generates interrupts

1. Add interrupt controller integration for input device IRQ
2. Update input device to assert/clear IRQ line
3. Write `demos/irq_test.asm`: uses IRQ handler to read input
4. Test interrupt-driven input flow

**Deliverables**:
- IRQ-driven input demo works

---

### Phase 7: Asteroids Prototype
**Goal**: Minimal playable Asteroids

1. Write game loop: ship physics, rotation, thrust
2. Asteroid spawning and collision detection
3. Shooting mechanics
4. Score display (simple number rendering with lines)

**Deliverables**:
- Playable Asteroids prototype

---

## Open Questions

1. **Color model**: RGB332 (256 colors) or 4-bit palette (16 colors)? RGB332 is simpler.

2. **VGC busy flag**: Should commands block (busy flag) or execute instantly? Instant is simpler, blocking is more "realistic."

3. **Command buffer vs streaming**: Streaming registers (write opcode, args, exec) or buffered display list? Streaming is simpler for CPU, buffer is more powerful.

4. **IRQ priorities**: If both input and VGC can IRQ, what's the priority? Single IRQ line or separate vectors?

5. **Screen coordinates**: Top-left (0,0) or bottom-left like OpenGL? Top-left matches SDL and is more intuitive.

6. **Cycles per frame**: How to balance? 10,000 cycles = 166 KHz at 60 FPS. Is that enough for game logic?

7. **Sound**: Defer entirely, or plan MMIO sound device interface now? Probably defer.

8. **Trace on crash**: Should demo runner automatically dump trace buffer and debug info on halt? Yes, very useful.

9. **Window size**: Fixed 512x512 or configurable? Configurable via `--scale` flag.

10. **SDL version**: SDL2 (stable) or SDL3 (newer API)? SDL2 for now.

---

## Next Steps

1. Review and refine this spec
2. Answer open questions
3. Start Phase 1 implementation

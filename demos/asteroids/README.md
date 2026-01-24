# Asteroids Demo

A simple Asteroids-style game demonstrating modular assembly programming for the IRATA2 simulator.

## Controls

| Key | Action |
|-----|--------|
| Left Arrow | Rotate counter-clockwise (hold for continuous) |
| Right Arrow | Rotate clockwise (hold for continuous) |
| Up Arrow | Thrust/accelerate (hold for continuous) |
| Space | Fire bullet |

## Running

```bash
# Assemble
cd demos/asteroids
../../build/assembler/irata2_asm main.asm asteroids.bin asteroids.json

# Run with SDL frontend
../../build/frontend/irata2_demo --rom asteroids.bin --fps 30 --scale 2
```

## Current Features

### Ship
- 16-direction rotation (22.5 degrees per step)
- Velocity-based movement with momentum
- Thrust applies acceleration in facing direction
- Visual thrust flame when accelerating
- Screen wrapping (exits one side, enters opposite)

### Bullet
- Fires from ship position in ship direction
- Inherits ship velocity for realistic physics
- Limited lifetime (48 frames)
- One bullet at a time

### Asteroid
- Static square asteroid (placeholder)
- Fixed position at (48, 48)

### Input System
- Continuous input via KEY_STATE register (bitmask of held keys)
- Rate-limited rotation (~15 rotations/second)
- Rate-limited thrust for smooth acceleration
- Fire cooldown based on bullet lifetime

## Module Architecture

```
main.asm           Entry point and game loop
  |
  +-- hardware.asm    VGC and input register constants
  +-- variables.asm   Zero-page variable definitions
  +-- graphics.asm    Low-level VGC drawing subroutines
  +-- input.asm       Input polling and key detection
  +-- ship.asm        Ship state, physics, and rendering
  +-- bullet.asm      Bullet state, lifecycle, and rendering
  +-- asteroid.asm    Asteroid state and rendering
  +-- tables.asm      Direction and velocity lookup tables
```

### Module Interfaces

#### graphics.asm
| Subroutine | Description |
|------------|-------------|
| `gfx_clear` | Clear screen to black |
| `gfx_point` | Draw point at (draw_x, draw_y) |
| `gfx_line` | Draw line from (draw_x0, draw_y0) to (draw_x1, draw_y1) |
| `gfx_frame` | Signal frame complete to VGC |

#### input.asm
| Subroutine | Description |
|------------|-------------|
| `input_poll` | Poll input, Z=1 if no input |
| `input_is_left` | Z=1 if left arrow held |
| `input_is_right` | Z=1 if right arrow held |
| `input_is_thrust` | Z=1 if up arrow held |
| `input_is_fire` | Z=1 if space held |

#### ship.asm
| Subroutine | Description |
|------------|-------------|
| `ship_init` | Initialize ship at center |
| `ship_rotate_ccw` | Rotate counter-clockwise |
| `ship_rotate_cw` | Rotate clockwise |
| `ship_apply_thrust` | Apply thrust in facing direction |
| `ship_update` | Update position from velocity |
| `ship_draw` | Draw ship triangle and flame |

#### bullet.asm
| Subroutine | Description |
|------------|-------------|
| `bullet_init` | Initialize bullet as inactive |
| `bullet_fire` | Fire bullet from ship |
| `bullet_update` | Update position, handle lifetime |
| `bullet_draw` | Draw bullet if active |

#### asteroid.asm
| Subroutine | Description |
|------------|-------------|
| `asteroid_init` | Initialize asteroid position |
| `asteroid_draw` | Draw asteroid square |

## Memory Map (Zero Page)

| Range | Purpose |
|-------|---------|
| $00-$0F | Ship state (position, velocity, angle, timers) |
| $10-$1F | Bullet state |
| $18-$1F | Asteroid state |
| $20-$2F | Scratch/temp variables |
| $28-$2C | Draw parameters (coordinates, color) |
| $30-$3F | Ship drawing computed values |

## Design Philosophy

1. **Modular**: Each game system is in its own file with a defined interface
2. **Subroutine-based**: Reusable subroutines with documented inputs/outputs
3. **Constants for readability**: Named constants via `.equ` for hardware registers and magic numbers
4. **Table-driven**: Direction and velocity values in lookup tables for efficiency
5. **Rate-limited input**: Frame-based timers prevent excessive rotation/thrust

## Tuning Constants

Located in `main.asm`:
```asm
.equ ROTATE_DELAY,  $04     ; Frames between rotations
.equ THRUST_DELAY,  $02     ; Frames between thrust applications
```

Located in `bullet.asm`:
```asm
.equ BULLET_LIFETIME, $30   ; Bullet lifetime in frames (48)
```

## Roadmap

### Phase 0: Infrastructure
- [x] **Key state register** - KEY_STATE ($4005) MMIO register with bitmask of held keys
  - Bit 0 = up, Bit 1 = down, Bit 2 = left, Bit 3 = right, Bit 4 = space
- [ ] **Unit tests as asm files** - Create tests/ subdirectory with .asm test files
  - Run as part of integration test suite
  - Test individual module features in isolation
- [ ] **Rendering tests** - Graphics module tests that verify rendered output
  - Generate images from VGC output
  - Compare against expected reference images

### Phase 1: Core Gameplay
- [x] Ship movement with momentum
- [x] 16-direction rotation
- [x] Thrust with visual flame
- [x] Single bullet firing
- [x] Continuous input handling via KEY_STATE register
- [x] Rate-limited controls
- [ ] **Slower acceleration** - Reduce thrust table values for smoother control
- [ ] **More accurate angular movement** - Movement velocity respects ship direction in all 16 angles
- [ ] Asteroid collision detection
- [ ] Bullet-asteroid collision

### Phase 2: Multiple Objects
- [ ] Multiple asteroids
- [ ] Asteroid movement
- [ ] Asteroid splitting on hit
- [ ] Multiple bullets
- [ ] Score tracking

### Phase 3: Polish
- [ ] Asteroid spawning system
- [ ] Ship respawn/lives
- [ ] Sound effects (if supported)
- [ ] High score

### Phase 4: Advanced
- [ ] UFO enemy
- [ ] Hyperspace/teleport
- [ ] Screen shake on collision
- [ ] Particle effects

## Testing

Unit tests are in `assembler/test/integration/asteroids_modules_test.cpp`:
- Graphics clear screen verification
- Graphics point drawing
- Graphics line drawing
- Ship rotation logic
- Bullet lifecycle
- Asteroid square rendering
- Ship triangle rendering

Run tests:
```bash
./build/assembler/test/integration_tests --gtest_filter="AsteroidsModulesTest.*"
```

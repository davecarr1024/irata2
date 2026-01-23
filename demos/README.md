# IRATA2 Demo Programs

These programs target the demo surface (input device + VGC).

## Build

```bash
./build/assembler/irata2_asm demos/blink.asm demos/blink.bin demos/blink.json
./build/assembler/irata2_asm demos/move_sprite.asm demos/move_sprite.bin demos/move_sprite.json
./build/assembler/irata2_asm demos/asteroids.asm demos/asteroids.bin demos/asteroids.json
```

## Run (SDL frontend)

Build the frontend with `-DIRATA2_ENABLE_SDL=ON`, then:

```bash
./build/frontend/irata2_demo --rom demos/blink.bin --fps 30 --scale 2
```

## Controls

- Arrow keys: movement
- Space: action (unused in these demos)

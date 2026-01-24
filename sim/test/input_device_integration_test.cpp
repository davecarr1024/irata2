#include "irata2/assembler/assembler.h"
#include "irata2/sim.h"
#include "irata2/sim/io/input_device.h"

#include <gtest/gtest.h>

#include <memory>

using irata2::assembler::AssemblerResult;
using irata2::assembler::Assemble;
using irata2::base::Byte;
using irata2::base::Word;
using irata2::sim::Cpu;
using irata2::sim::DefaultHdl;
using irata2::sim::DefaultMicrocodeProgram;
using irata2::sim::LatchedProcessControl;
using irata2::sim::io::InputDevice;
using irata2::sim::io::INPUT_DEVICE_BASE;
using irata2::sim::memory::Memory;
using irata2::sim::memory::Region;

namespace {
struct CpuWithInput {
  std::unique_ptr<Cpu> cpu;
  InputDevice* device = nullptr;
};

CpuWithInput MakeCpuWithInputDevice(const std::vector<Byte>& rom) {
  CpuWithInput result;

  std::vector<Memory::RegionFactory> factories;
  factories.push_back([&result](Memory& mem,
                                LatchedProcessControl& irq_line)
                           -> std::unique_ptr<Region> {
    return std::make_unique<Region>(
        "input_device", mem, Word{INPUT_DEVICE_BASE},
        [&result, &irq_line](Region& region)
            -> std::unique_ptr<irata2::sim::memory::Module> {
          auto device = std::make_unique<InputDevice>("input", region, irq_line);
          result.device = device.get();
          return device;
        });
  });

  result.cpu = std::make_unique<Cpu>(
      DefaultHdl(), DefaultMicrocodeProgram(), rom, std::move(factories));
  return result;
}

void InitializeCpu(Cpu& cpu, Word entry) {
  cpu.pc().set_value(entry);
  cpu.controller().sc().set_value(Byte{0});
  cpu.controller().ir().set_value(cpu.memory().ReadAt(entry));
}
}  // namespace

TEST(InputDeviceIntegrationTest, ProgramReadsInputAndStoresToRam) {
  const std::string program = R"(
    LDA $4000
    AND #$01
    BEQ wait
    LDA $4002
    STA $0200
    HLT
  wait:
    JMP wait
  )";

  AssemblerResult assembled = Assemble(program, "input_device_integration.asm");
  std::vector<Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(Byte{value});
  }

  CpuWithInput rig = MakeCpuWithInputDevice(rom);
  ASSERT_NE(rig.device, nullptr);

  rig.device->inject_key(0x41);
  InitializeCpu(*rig.cpu, assembled.header.entry);

  auto result = rig.cpu->RunUntilHalt(2000);
  ASSERT_EQ(result.reason, Cpu::HaltReason::Halt);

  Byte stored = rig.cpu->memory().ReadAt(Word{0x0200});
  EXPECT_EQ(stored.value(), 0x41);
}

TEST(InputDeviceIntegrationTest, KeyStateRegisterForContinuousInput) {
  // Test that KEY_STATE ($4005) can be read by the CPU and reflects
  // the key state set via set_key_down/set_key_up.
  const std::string program = R"(
    ; Read KEY_STATE and store it
    LDA $4005
    STA $0200
    HLT
  )";

  AssemblerResult assembled = Assemble(program, "key_state_integration.asm");
  std::vector<Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(Byte{value});
  }

  CpuWithInput rig = MakeCpuWithInputDevice(rom);
  ASSERT_NE(rig.device, nullptr);

  // Set LEFT key down (bit 2 = 0x04)
  rig.device->set_key_down(irata2::sim::io::key_state_bits::LEFT);
  InitializeCpu(*rig.cpu, assembled.header.entry);

  auto result = rig.cpu->RunUntilHalt(2000);
  ASSERT_EQ(result.reason, Cpu::HaltReason::Halt);

  // The program should have read KEY_STATE and stored it at $0200
  Byte stored = rig.cpu->memory().ReadAt(Word{0x0200});
  EXPECT_EQ(stored.value(), irata2::sim::io::key_state_bits::LEFT);
}

TEST(InputDeviceIntegrationTest, KeyStatePersistsAcrossMultipleReads) {
  // Test that KEY_STATE maintains its value across multiple CPU reads
  // (simulating multiple game loop iterations)
  const std::string program = R"(
    ; Read KEY_STATE multiple times and count how many times it's non-zero
    LDA #$00
    STA $0200           ; counter = 0

    ; Loop 10 times, incrementing counter if KEY_STATE has LEFT bit set
    LDX #$0A            ; loop 10 times
  loop:
    LDA $4005           ; read KEY_STATE
    AND #$04            ; check LEFT bit
    BEQ skip
    INC $0200           ; increment counter if LEFT held
  skip:
    DEX
    BNE loop
    HLT
  )";

  AssemblerResult assembled = Assemble(program, "key_state_persistent.asm");
  std::vector<Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(Byte{value});
  }

  CpuWithInput rig = MakeCpuWithInputDevice(rom);
  ASSERT_NE(rig.device, nullptr);

  // Set LEFT key down - should persist for all 10 reads
  rig.device->set_key_down(irata2::sim::io::key_state_bits::LEFT);
  InitializeCpu(*rig.cpu, assembled.header.entry);

  auto result = rig.cpu->RunUntilHalt(5000);
  ASSERT_EQ(result.reason, Cpu::HaltReason::Halt);

  // Counter should be 10 (LEFT was held for all 10 iterations)
  Byte counter = rig.cpu->memory().ReadAt(Word{0x0200});
  EXPECT_EQ(counter.value(), 10) << "KEY_STATE should persist and be readable 10 times";
}

TEST(InputDeviceIntegrationTest, KeyStateReflectsKeyUp) {
  // Test that KEY_STATE correctly reflects key release
  const std::string program = R"(
    ; Read KEY_STATE and store it at $0200
    LDA $4005
    STA $0200
    ; Signal we've done first read by storing $FF at $0201
    LDA #$FF
    STA $0201
    ; Wait loop - external test will call set_key_up
  wait:
    LDA $0202           ; check if test has signaled to continue
    BEQ wait
    ; Read KEY_STATE again after key up
    LDA $4005
    STA $0203
    HLT
  )";

  AssemblerResult assembled = Assemble(program, "key_state_keyup.asm");
  std::vector<Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(Byte{value});
  }

  CpuWithInput rig = MakeCpuWithInputDevice(rom);
  ASSERT_NE(rig.device, nullptr);

  // Set LEFT key down initially
  rig.device->set_key_down(irata2::sim::io::key_state_bits::LEFT);
  InitializeCpu(*rig.cpu, assembled.header.entry);

  // Run until the program stores $FF at $0201 (first read done)
  for (int i = 0; i < 10000; ++i) {
    rig.cpu->Tick();
    if (rig.cpu->memory().ReadAt(Word{0x0201}).value() == 0xFF) {
      break;
    }
  }
  ASSERT_EQ(rig.cpu->memory().ReadAt(Word{0x0201}).value(), 0xFF)
      << "Program should have completed first KEY_STATE read";

  // First read should have LEFT set
  EXPECT_EQ(rig.cpu->memory().ReadAt(Word{0x0200}).value(),
            irata2::sim::io::key_state_bits::LEFT);

  // Now release the key
  rig.device->set_key_up(irata2::sim::io::key_state_bits::LEFT);

  // Signal program to continue by writing to $0202
  rig.cpu->memory().WriteAt(Word{0x0202}, Byte{0x01});

  // Run until halt
  auto result = rig.cpu->RunUntilHalt(5000);
  ASSERT_EQ(result.reason, Cpu::HaltReason::Halt);

  // Second read (at $0203) should have LEFT cleared
  Byte second_read = rig.cpu->memory().ReadAt(Word{0x0203});
  EXPECT_EQ(second_read.value(), 0) << "KEY_STATE should be 0 after key release";
}

TEST(InputDeviceIntegrationTest, RateLimitedRotationWithKeyHeld) {
  // This test simulates the actual asteroids game loop:
  // - Poll KEY_STATE
  // - If key held AND timer == 0: rotate and reset timer
  // - Decrement timer each iteration
  // - After enough iterations, rotation should happen again
  //
  // Variables:
  //   $00 = timer
  //   $01 = angle
  //   $02 = rotation_count
  const std::string program = R"(
    .equ KEY_STATE, $4005
    .equ STATE_LEFT, $04
    .equ RATE_DELAY, $04    ; Same as ROTATE_DELAY in asteroids

    ; Initialize
    LDA #$00
    STA $00                 ; timer = 0
    STA $02                 ; rotation_count = 0
    LDA #$08
    STA $01                 ; angle = 8

    ; Run 20 loop iterations (should get ~4 rotations at rate=4)
    LDX #$14                ; 20 iterations

  main_loop:
    ; --- Decrement timer if > 0 ---
    LDA $00
    BEQ timer_done
    SEC
    SBC #$01
    STA $00
  timer_done:

    ; --- Poll KEY_STATE ---
    LDA KEY_STATE
    AND #STATE_LEFT
    BEQ loop_end            ; No left key, skip rotation check

    ; --- Check timer for rotation ---
    LDA $00
    BNE loop_end            ; Timer > 0, skip rotation

    ; --- Rotate! ---
    DEC $01                 ; angle--
    INC $02                 ; rotation_count++
    LDA #RATE_DELAY
    STA $00                 ; Reset timer

  loop_end:
    DEX
    BNE main_loop
    HLT
  )";

  AssemblerResult assembled = Assemble(program, "rate_limited_rotation.asm");
  std::vector<Byte> rom;
  rom.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom.push_back(Byte{value});
  }

  CpuWithInput rig = MakeCpuWithInputDevice(rom);
  ASSERT_NE(rig.device, nullptr);

  // Set LEFT key down before starting
  rig.device->set_key_down(irata2::sim::io::key_state_bits::LEFT);
  InitializeCpu(*rig.cpu, assembled.header.entry);

  auto result = rig.cpu->RunUntilHalt(50000);
  ASSERT_EQ(result.reason, Cpu::HaltReason::Halt);

  // With 20 iterations and rate=4:
  // Iteration 1: timer=0, rotate, count=1, timer=4
  // Iteration 2: timer 4->3, skip
  // Iteration 3: timer 3->2, skip
  // Iteration 4: timer 2->1, skip
  // Iteration 5: timer 1->0, rotate, count=2, timer=4
  // Iteration 9: rotate, count=3
  // Iteration 13: rotate, count=4
  // Iteration 17: rotate, count=5
  // So we expect 5 rotations (at iterations 1, 5, 9, 13, 17)

  Byte rotation_count = rig.cpu->memory().ReadAt(Word{0x0002});
  EXPECT_EQ(rotation_count.value(), 5)
      << "Should have 5 rotations in 20 iterations with rate=4";

  Byte final_angle = rig.cpu->memory().ReadAt(Word{0x0001});
  EXPECT_EQ(final_angle.value(), 3)  // Started at 8, rotated 5 times
      << "Angle should be 8 - 5 = 3";
}

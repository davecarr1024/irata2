#ifndef IRATA2_SIM_IO_INPUT_DEVICE_H
#define IRATA2_SIM_IO_INPUT_DEVICE_H

#include <array>
#include <cstddef>
#include <cstdint>

#include "irata2/base/types.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/memory/module.h"

namespace irata2::sim::io {

/// Input device base address in memory map.
/// Mapped at $4000 in the MMIO region (between RAM at $0000 and ROM at $8000).
constexpr uint16_t INPUT_DEVICE_BASE = 0x4000;

/// Input device MMIO register offsets (relative to INPUT_DEVICE_BASE).
namespace input_reg {
constexpr uint8_t STATUS = 0x00;    // R: bit 0=not empty, bit 1=full, bit 7=IRQ pending
constexpr uint8_t CONTROL = 0x01;   // W: bit 0=enable IRQ
constexpr uint8_t DATA = 0x02;      // R: read and pop from queue
constexpr uint8_t PEEK = 0x03;      // R: read without popping
constexpr uint8_t COUNT = 0x04;     // R: number of bytes in queue (0-16)
constexpr uint8_t KEY_STATE = 0x05; // R: bitmask of currently held keys
}  // namespace input_reg

/// Key state bitmask bit positions.
namespace key_state_bits {
constexpr uint8_t UP = 0x01;    // Bit 0: up arrow held
constexpr uint8_t DOWN = 0x02;  // Bit 1: down arrow held
constexpr uint8_t LEFT = 0x04;  // Bit 2: left arrow held
constexpr uint8_t RIGHT = 0x08; // Bit 3: right arrow held
constexpr uint8_t SPACE = 0x10; // Bit 4: space held
}  // namespace key_state_bits

/// Status register bit positions.
namespace input_status {
constexpr uint8_t NOT_EMPTY = 0x01;   // Bit 0: queue not empty
constexpr uint8_t FULL = 0x02;        // Bit 1: queue full
constexpr uint8_t IRQ_PENDING = 0x80; // Bit 7: IRQ pending
}  // namespace input_status

/// Control register bit positions.
namespace input_control {
constexpr uint8_t IRQ_ENABLE = 0x01;  // Bit 0: enable IRQ on input
}  // namespace input_control

/// Input device with 16-byte keyboard queue and MMIO registers.
///
/// This device accepts keyboard input from the frontend, buffers it in a
/// circular queue, and exposes it to the CPU via memory-mapped I/O.
///
/// MMIO Map (16 bytes at $4000-$400F):
///   $4000 STATUS  (R)  - Status flags
///   $4001 CONTROL (W)  - Control register
///   $4002 DATA    (R)  - Read and pop from queue
///   $4003 PEEK    (R)  - Read without popping
///   $4004 COUNT   (R)  - Bytes in queue (0-16)
///   $4005-$400F        - Reserved
///
/// @see docs/projects/demo-surface.md for full specification
class InputDevice final : public memory::Module {
 public:
  static constexpr size_t QUEUE_SIZE = 16;
  static constexpr size_t MMIO_SIZE = 16;  // Power of 2, aligned

  InputDevice(std::string name, Component& parent, LatchedProcessControl& irq_line);

  // Module interface
  size_t size() const override { return MMIO_SIZE; }
  base::Byte Read(base::Word address) const override;
  void Write(base::Word address, base::Byte value) override;

  // Frontend interface - inject key codes into the queue
  void inject_key(uint8_t key_code);

  // Frontend interface - set/clear key state bits for continuous input
  void set_key_down(uint8_t bit);
  void set_key_up(uint8_t bit);

  // Key state query (for testing)
  uint8_t key_state() const { return key_state_; }

  // IRQ interface
  bool irq_pending() const { return irq_enabled_ && !empty(); }

  // Queue state (for testing)
  bool empty() const { return count_ == 0; }
  bool full() const { return count_ == QUEUE_SIZE; }
  size_t count() const { return count_; }

 private:
  std::array<uint8_t, QUEUE_SIZE> queue_{};
  size_t read_idx_ = 0;
  size_t write_idx_ = 0;
  size_t count_ = 0;
  bool irq_enabled_ = false;
  uint8_t key_state_ = 0;  // Bitmask of currently held keys
  LatchedProcessControl& irq_line_;

  uint8_t pop();
  uint8_t peek() const;

  void TickControl() override;
};

}  // namespace irata2::sim::io

#endif  // IRATA2_SIM_IO_INPUT_DEVICE_H

#ifndef IRATA2_SIM_IO_VECTOR_GRAPHICS_COPROCESSOR_H
#define IRATA2_SIM_IO_VECTOR_GRAPHICS_COPROCESSOR_H

#include <cstddef>
#include <cstdint>
#include <memory>

#include "irata2/base/types.h"
#include "irata2/sim/memory/module.h"
#include "irata2/sim/io/vgc_backend.h"

namespace irata2::sim::io {

constexpr uint16_t VGC_BASE = 0x4100;

namespace vgc_reg {
constexpr uint8_t CMD = 0x00;
constexpr uint8_t X0 = 0x01;
constexpr uint8_t Y0 = 0x02;
constexpr uint8_t X1 = 0x03;
constexpr uint8_t Y1 = 0x04;
constexpr uint8_t COLOR = 0x05;
constexpr uint8_t EXEC = 0x06;
constexpr uint8_t CONTROL = 0x07;
constexpr uint8_t STATUS = 0x08;
}  // namespace vgc_reg

namespace vgc_status {
constexpr uint8_t BUSY = 0x01;
constexpr uint8_t IRQ_PENDING = 0x80;
}  // namespace vgc_status

namespace vgc_control {
constexpr uint8_t CLEAR = 0x01;
constexpr uint8_t PRESENT = 0x02;
constexpr uint8_t IRQ_ENABLE = 0x80;
}  // namespace vgc_control

namespace vgc_cmd {
constexpr uint8_t NOP = 0x00;
constexpr uint8_t CLEAR = 0x01;
constexpr uint8_t POINT = 0x02;
constexpr uint8_t LINE = 0x03;
}  // namespace vgc_cmd

class VectorGraphicsCoprocessor final : public memory::Module {
 public:
  static constexpr size_t MMIO_SIZE = 16;

  VectorGraphicsCoprocessor(std::string name,
                            Component& parent,
                            std::unique_ptr<VgcBackend> backend);

  size_t size() const override { return MMIO_SIZE; }
  base::Byte Read(base::Word address) const override;
  void Write(base::Word address, base::Byte value) override;

  VgcBackend& backend() { return *backend_; }
  const VgcBackend& backend() const { return *backend_; }

 private:
  std::unique_ptr<VgcBackend> backend_;
  uint8_t cmd_ = 0;
  uint8_t x0_ = 0;
  uint8_t y0_ = 0;
  uint8_t x1_ = 0;
  uint8_t y1_ = 0;
  uint8_t color_ = 0;
  bool irq_enabled_ = false;

  uint8_t intensity() const { return static_cast<uint8_t>(color_ & 0x03); }
  void ExecuteCommand();
  void ApplyControl(uint8_t control);
};

}  // namespace irata2::sim::io

#endif  // IRATA2_SIM_IO_VECTOR_GRAPHICS_COPROCESSOR_H

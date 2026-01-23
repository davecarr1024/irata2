#include "irata2/sim/io/vector_graphics_coprocessor.h"

#include "irata2/sim/error.h"

namespace irata2::sim::io {

VectorGraphicsCoprocessor::VectorGraphicsCoprocessor(
    std::string name,
    Component& parent,
    std::unique_ptr<VgcBackend> backend)
    : Module(std::move(name), parent),
      backend_(std::move(backend)) {
  if (!backend_) {
    throw SimError("VGC backend is null");
  }
}

base::Byte VectorGraphicsCoprocessor::Read(base::Word address) const {
  const auto offset = address.value();
  switch (offset) {
    case vgc_reg::STATUS:
      return base::Byte{0};
    default:
      return base::Byte{0};
  }
}

void VectorGraphicsCoprocessor::Write(base::Word address, base::Byte value) {
  const auto offset = address.value();
  const uint8_t raw = value.value();

  switch (offset) {
    case vgc_reg::CMD:
      cmd_ = raw;
      break;
    case vgc_reg::X0:
      x0_ = raw;
      break;
    case vgc_reg::Y0:
      y0_ = raw;
      break;
    case vgc_reg::X1:
      x1_ = raw;
      break;
    case vgc_reg::Y1:
      y1_ = raw;
      break;
    case vgc_reg::COLOR:
      color_ = raw;
      break;
    case vgc_reg::EXEC:
      if (raw == 0x01) {
        ExecuteCommand();
      }
      break;
    case vgc_reg::CONTROL:
      ApplyControl(raw);
      break;
    default:
      break;
  }
}

void VectorGraphicsCoprocessor::ExecuteCommand() {
  switch (cmd_) {
    case vgc_cmd::NOP:
      break;
    case vgc_cmd::CLEAR:
      backend_->clear(intensity());
      break;
    case vgc_cmd::POINT:
      backend_->draw_point(x0_, y0_, intensity());
      break;
    case vgc_cmd::LINE:
      backend_->draw_line(x0_, y0_, x1_, y1_, intensity());
      break;
    default:
      break;
  }
}

void VectorGraphicsCoprocessor::ApplyControl(uint8_t control) {
  irq_enabled_ = (control & vgc_control::IRQ_ENABLE) != 0;
  if (control & vgc_control::CLEAR) {
    backend_->clear(intensity());
  }
  if (control & vgc_control::PRESENT) {
    backend_->present();
  }
}

}  // namespace irata2::sim::io

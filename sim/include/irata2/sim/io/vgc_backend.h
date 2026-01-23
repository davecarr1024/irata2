#ifndef IRATA2_SIM_IO_VGC_BACKEND_H
#define IRATA2_SIM_IO_VGC_BACKEND_H

#include <array>
#include <cstdint>
#include <cstddef>

namespace irata2::sim::io {

class VgcBackend {
 public:
  virtual ~VgcBackend() = default;

  virtual void clear(uint8_t intensity) = 0;
  virtual void draw_point(uint8_t x, uint8_t y, uint8_t intensity) = 0;
  virtual void draw_line(uint8_t x0,
                         uint8_t y0,
                         uint8_t x1,
                         uint8_t y1,
                         uint8_t intensity) = 0;
  virtual void present() = 0;
};

class ImageBackend final : public VgcBackend {
 public:
  static constexpr size_t kWidth = 256;
  static constexpr size_t kHeight = 256;

  void clear(uint8_t intensity) override;
  void draw_point(uint8_t x, uint8_t y, uint8_t intensity) override;
  void draw_line(uint8_t x0,
                 uint8_t y0,
                 uint8_t x1,
                 uint8_t y1,
                 uint8_t intensity) override;
  void present() override {}

  const std::array<uint8_t, kWidth * kHeight>& framebuffer() const {
    return framebuffer_;
  }

 private:
  std::array<uint8_t, kWidth * kHeight> framebuffer_{};
};

}  // namespace irata2::sim::io

#endif  // IRATA2_SIM_IO_VGC_BACKEND_H

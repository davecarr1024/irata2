#include "irata2/sim/io/vgc_backend.h"

#include <algorithm>
#include <cstdlib>

namespace irata2::sim::io {

void ImageBackend::clear(uint8_t intensity) {
  std::fill(framebuffer_.begin(), framebuffer_.end(), intensity);
}

void ImageBackend::draw_point(uint8_t x, uint8_t y, uint8_t intensity) {
  const size_t px = static_cast<size_t>(x);
  const size_t py = static_cast<size_t>(y);
  if (px >= kWidth || py >= kHeight) {
    return;
  }
  framebuffer_[py * kWidth + px] = intensity;
}

void ImageBackend::draw_line(uint8_t x0,
                             uint8_t y0,
                             uint8_t x1,
                             uint8_t y1,
                             uint8_t intensity) {
  int x = x0;
  int y = y0;
  const int target_x = x1;
  const int target_y = y1;

  const int dx = std::abs(target_x - x);
  const int dy = -std::abs(target_y - y);
  const int step_x = (x < target_x) ? 1 : -1;
  const int step_y = (y < target_y) ? 1 : -1;
  int error = dx + dy;

  while (true) {
    draw_point(static_cast<uint8_t>(x),
               static_cast<uint8_t>(y),
               intensity);
    if (x == target_x && y == target_y) {
      break;
    }
    const int error2 = 2 * error;
    if (error2 >= dy) {
      error += dy;
      x += step_x;
    }
    if (error2 <= dx) {
      error += dx;
      y += step_y;
    }
  }
}

}  // namespace irata2::sim::io

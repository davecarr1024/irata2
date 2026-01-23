#include "irata2/sim/io/vgc_backend.h"

#include <gtest/gtest.h>

using irata2::sim::io::ImageBackend;

TEST(ImageBackendTest, ClearFillsFramebuffer) {
  ImageBackend backend;
  backend.clear(0x03);

  const auto& fb = backend.framebuffer();
  for (uint8_t value : fb) {
    EXPECT_EQ(value, 0x03);
  }
}

TEST(ImageBackendTest, DrawPointWritesPixel) {
  ImageBackend backend;
  backend.clear(0x00);
  backend.draw_point(10, 20, 0x02);

  const auto& fb = backend.framebuffer();
  EXPECT_EQ(fb[20 * ImageBackend::kWidth + 10], 0x02);
}

TEST(ImageBackendTest, DrawLineTouchesEndpoints) {
  ImageBackend backend;
  backend.clear(0x00);
  backend.draw_line(0, 0, 5, 5, 0x01);

  const auto& fb = backend.framebuffer();
  EXPECT_EQ(fb[0], 0x01);
  EXPECT_EQ(fb[5 * ImageBackend::kWidth + 5], 0x01);
}

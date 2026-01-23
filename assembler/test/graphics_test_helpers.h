#ifndef IRATA2_ASSEMBLER_TEST_GRAPHICS_TEST_HELPERS_H
#define IRATA2_ASSEMBLER_TEST_GRAPHICS_TEST_HELPERS_H

#include <cstdint>
#include <string>

#include "irata2/sim/io/vgc_backend.h"

namespace irata2::assembler::test {

// Execute assembly source code with a VGC ImageBackend and return the
// resulting framebuffer. The program runs until it halts or hits max_cycles.
sim::io::ImageBackend ExecuteAndRender(const std::string& source,
                                       const std::string& filename = "test.asm",
                                       size_t max_cycles = 100000);

// Assertion helpers for verifying graphics output

// Expect pixel at (x, y) to have the given intensity
void ExpectPixel(const sim::io::ImageBackend& backend,
                 uint8_t x, uint8_t y, uint8_t intensity);

// Expect all pixels in the framebuffer to have the given intensity (clear screen test)
void ExpectClear(const sim::io::ImageBackend& backend, uint8_t intensity);

// Count how many pixels have the given intensity
size_t CountPixels(const sim::io::ImageBackend& backend, uint8_t intensity);

// Dump framebuffer as ASCII art for debugging (0-3 intensity levels)
std::string DumpFramebuffer(const sim::io::ImageBackend& backend);

}  // namespace irata2::assembler::test

#endif  // IRATA2_ASSEMBLER_TEST_GRAPHICS_TEST_HELPERS_H

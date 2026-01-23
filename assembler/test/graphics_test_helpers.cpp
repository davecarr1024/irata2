#include "graphics_test_helpers.h"

#include <sstream>
#include <memory>

#include <gtest/gtest.h>

#include "irata2/assembler/assembler.h"
#include "irata2/sim/cpu.h"
#include "irata2/sim/initialization.h"
#include "irata2/sim/io/vector_graphics_coprocessor.h"
#include "irata2/sim/memory/memory.h"
#include "irata2/sim/memory/region.h"
#include "irata2/sim/memory/module.h"

namespace irata2::assembler::test {

sim::io::ImageBackend ExecuteAndRender(const std::string& source,
                                       const std::string& filename,
                                       size_t max_cycles) {
  // Assemble the source
  AssemblerResult result = Assemble(source, filename);

  // Convert ROM to Byte vector
  std::vector<base::Byte> rom;
  rom.reserve(result.rom.size());
  for (uint8_t value : result.rom) {
    rom.push_back(base::Byte{value});
  }

  // Create region factory for VGC
  sim::io::ImageBackend* backend_ptr = nullptr;
  std::vector<sim::memory::Memory::RegionFactory> factories;

  factories.push_back([&backend_ptr](sim::memory::Memory& mem,
                                     sim::LatchedProcessControl&)
                          -> std::unique_ptr<sim::memory::Region> {
    return std::make_unique<sim::memory::Region>(
        "vgc", mem, base::Word{0x4100},
        [&backend_ptr](sim::memory::Region& region)
            -> std::unique_ptr<sim::memory::Module> {
          auto backend = std::make_unique<sim::io::ImageBackend>();
          backend_ptr = backend.get();
          auto vgc = std::make_unique<sim::io::VectorGraphicsCoprocessor>(
              "vgc", region, std::move(backend));
          return vgc;
        });
  });

  // Create CPU with VGC
  sim::Cpu cpu(sim::DefaultHdl(), sim::DefaultMicrocodeProgram(),
               rom, std::move(factories));

  // Initialize CPU
  cpu.pc().set_value(result.header.entry);
  cpu.controller().sc().set_value(base::Byte{0});
  cpu.controller().ir().set_value(cpu.memory().ReadAt(result.header.entry));

  // Run until halt
  auto run_result = cpu.RunUntilHalt(max_cycles, false);

  // Return the backend (copy it out before CPU is destroyed)
  EXPECT_NE(backend_ptr, nullptr) << "VGC backend was not created";
  if (backend_ptr) {
    return *backend_ptr;
  }
  return sim::io::ImageBackend();  // Return empty backend on error
}

void ExpectPixel(const sim::io::ImageBackend& backend,
                 uint8_t x, uint8_t y, uint8_t intensity) {
  const auto& fb = backend.framebuffer();
  size_t index = static_cast<size_t>(y) * 256 + static_cast<size_t>(x);
  EXPECT_LT(index, fb.size()) << "Pixel coordinates out of bounds: (" << static_cast<int>(x) << ", " << static_cast<int>(y) << ")";
  if (index < fb.size()) {
    EXPECT_EQ(fb[index], intensity) << "Pixel at (" << static_cast<int>(x) << ", " << static_cast<int>(y) << ")";
  }
}

void ExpectClear(const sim::io::ImageBackend& backend, uint8_t intensity) {
  const auto& fb = backend.framebuffer();
  for (size_t i = 0; i < fb.size(); ++i) {
    if (fb[i] != intensity) {
      uint8_t x = static_cast<uint8_t>(i % 256);
      uint8_t y = static_cast<uint8_t>(i / 256);
      FAIL() << "Expected all pixels to be " << static_cast<int>(intensity)
             << " but pixel at (" << static_cast<int>(x) << ", " << static_cast<int>(y)
             << ") was " << static_cast<int>(fb[i]);
    }
  }
}

size_t CountPixels(const sim::io::ImageBackend& backend, uint8_t intensity) {
  const auto& fb = backend.framebuffer();
  size_t count = 0;
  for (uint8_t pixel : fb) {
    if (pixel == intensity) {
      ++count;
    }
  }
  return count;
}

std::string DumpFramebuffer(const sim::io::ImageBackend& backend) {
  const auto& fb = backend.framebuffer();
  std::ostringstream out;

  // Use ASCII characters for different intensity levels
  const char chars[] = {' ', '.', '+', '#'};  // 0=black, 1=dim, 2=medium, 3=bright

  out << "Framebuffer (256x256):\n";
  out << "+";
  for (int x = 0; x < 256; ++x) out << "-";
  out << "+\n";

  for (int y = 0; y < 256; ++y) {
    out << "|";
    for (int x = 0; x < 256; ++x) {
      size_t index = static_cast<size_t>(y) * 256 + static_cast<size_t>(x);
      uint8_t intensity = fb[index];
      out << chars[intensity % 4];
    }
    out << "|\n";
  }

  out << "+";
  for (int x = 0; x < 256; ++x) out << "-";
  out << "+\n";

  return out.str();
}

}  // namespace irata2::assembler::test

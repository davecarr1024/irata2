#ifndef IRATA2_FRONTEND_DEMO_RUNNER_H
#define IRATA2_FRONTEND_DEMO_RUNNER_H

#include <cstdint>
#include <memory>
#include <string>

#include <SDL.h>

#include "irata2/base/types.h"
#include "irata2/sim/cpu.h"
#include "irata2/sim/io/input_device.h"
#include "irata2/sim/io/vector_graphics_coprocessor.h"

namespace irata2::frontend {

struct DemoOptions {
  std::string rom_path;
  int fps = 30;
  int scale = 2;
  int64_t cycles_per_frame = 0;
  bool debug_on_crash = false;
  size_t trace_size = 0;
};

class DemoRunner {
 public:
  explicit DemoRunner(DemoOptions options);
  ~DemoRunner();

  int Run();

 private:
  DemoOptions options_;
  std::unique_ptr<sim::Cpu> cpu_;
  sim::io::InputDevice* input_device_ = nullptr;
  sim::io::VectorGraphicsCoprocessor* vgc_ = nullptr;

  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;

  void HandleEvent(const SDL_Event& event);
  void TickCpu();
  void RenderFrame();
  void ShutdownSdl();

  static uint8_t MapKey(SDL_Keycode key);
};

}  // namespace irata2::frontend

#endif  // IRATA2_FRONTEND_DEMO_RUNNER_H

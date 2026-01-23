#include "irata2/frontend/demo_runner.h"

#include <chrono>
#include <optional>
#include <utility>
#include <vector>
#include <stdexcept>

#include "irata2/frontend/sdl_backend.h"
#include "irata2/sim/cartridge.h"
#include "irata2/sim/debug_dump.h"
#include "irata2/sim/initialization.h"

namespace irata2::frontend {

namespace {
struct DeviceBundle {
  sim::io::InputDevice* input = nullptr;
  sim::io::VectorGraphicsCoprocessor* vgc = nullptr;
};

void ResetCpu(sim::Cpu& cpu, base::Word entry) {
  cpu.pc().set_value(entry);
  cpu.controller().sc().set_value(base::Byte{0});
  cpu.controller().ir().set_value(cpu.memory().ReadAt(entry));
}
}  // namespace

DemoRunner::DemoRunner(DemoOptions options) : options_(std::move(options)) {
  if (options_.cycles_per_frame <= 0 && options_.fps > 0) {
    options_.cycles_per_frame = 100000 / options_.fps;
  }
  if (options_.fps <= 0 || options_.scale <= 0) {
    throw std::runtime_error("invalid demo options");
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    throw std::runtime_error(SDL_GetError());
  }
  window_ = SDL_CreateWindow("IRATA2 Demo",
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             256 * options_.scale,
                             256 * options_.scale,
                             SDL_WINDOW_SHOWN);
  if (!window_) {
    ShutdownSdl();
    throw std::runtime_error(SDL_GetError());
  }
  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer_) {
    ShutdownSdl();
    throw std::runtime_error(SDL_GetError());
  }
  SDL_RenderSetScale(renderer_,
                     static_cast<float>(options_.scale),
                     static_cast<float>(options_.scale));
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

  auto cartridge = sim::LoadCartridge(options_.rom_path);
  DeviceBundle bundle;
  std::vector<sim::memory::Memory::RegionFactory> factories;
  factories.push_back([&bundle](sim::memory::Memory& mem)
                          -> std::unique_ptr<sim::memory::Region> {
    return std::make_unique<sim::memory::Region>(
        "input_device", mem, base::Word{sim::io::INPUT_DEVICE_BASE},
        [&bundle](sim::memory::Region& region)
            -> std::unique_ptr<sim::memory::Module> {
          auto device = std::make_unique<sim::io::InputDevice>("input", region);
          bundle.input = device.get();
          return device;
        });
  });

  factories.push_back([&bundle, renderer = renderer_](sim::memory::Memory& mem)
                          -> std::unique_ptr<sim::memory::Region> {
    return std::make_unique<sim::memory::Region>(
        "vgc", mem, base::Word{sim::io::VGC_BASE},
        [&bundle, renderer](sim::memory::Region& region)
            -> std::unique_ptr<sim::memory::Module> {
          auto backend = std::make_unique<SdlBackend>(renderer);
          auto device = std::make_unique<sim::io::VectorGraphicsCoprocessor>(
              "vgc", region, std::move(backend));
          bundle.vgc = device.get();
          return device;
        });
  });

  cpu_ = std::make_unique<sim::Cpu>(
      sim::DefaultHdl(),
      sim::DefaultMicrocodeProgram(),
      std::move(cartridge.rom),
      std::move(factories));
  input_device_ = bundle.input;
  vgc_ = bundle.vgc;

  ResetCpu(*cpu_, cartridge.header.entry);
  if (options_.trace_size > 0) {
    cpu_->EnableTrace(options_.trace_size);
  }
}

DemoRunner::~DemoRunner() {
  ShutdownSdl();
}

int DemoRunner::Run() {
  bool running = true;
  auto last_frame = std::chrono::steady_clock::now();

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
      HandleEvent(event);
    }

    TickCpu();
    RenderFrame();

    auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_frame);
    const auto frame_ms = std::chrono::milliseconds(1000 / options_.fps);
    if (elapsed < frame_ms) {
      SDL_Delay(static_cast<uint32_t>((frame_ms - elapsed).count()));
    }
    last_frame = std::chrono::steady_clock::now();

    if (cpu_->halted()) {
      running = false;
    }
  }

  return cpu_->crashed() ? 2 : 0;
}

void DemoRunner::HandleEvent(const SDL_Event& event) {
  if (event.type != SDL_KEYDOWN || event.key.repeat) {
    return;
  }

  const uint8_t code = MapKey(event.key.keysym.sym);
  if (code == 0x00) {
    return;
  }
  if (input_device_) {
    input_device_->inject_key(code);
  }
}

void DemoRunner::TickCpu() {
  auto result = cpu_->RunUntilHalt(
      static_cast<uint64_t>(options_.cycles_per_frame));
  if (result.reason == sim::Cpu::HaltReason::Crash && options_.debug_on_crash) {
    const std::string dump = sim::FormatDebugDump(*cpu_, "crash");
    SDL_Log("%s", dump.c_str());
  }
}

void DemoRunner::RenderFrame() {
  (void)vgc_;
}

void DemoRunner::ShutdownSdl() {
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
  SDL_Quit();
}

uint8_t DemoRunner::MapKey(SDL_Keycode key) {
  switch (key) {
    case SDLK_UP:
      return 0x01;
    case SDLK_DOWN:
      return 0x02;
    case SDLK_LEFT:
      return 0x03;
    case SDLK_RIGHT:
      return 0x04;
    case SDLK_SPACE:
      return 0x20;
    case SDLK_RETURN:
      return 0x0D;
    case SDLK_ESCAPE:
      return 0x1B;
    default:
      break;
  }

  if (key >= SDLK_0 && key <= SDLK_9) {
    return static_cast<uint8_t>(0x30 + (key - SDLK_0));
  }

  if (key >= SDLK_a && key <= SDLK_z) {
    return static_cast<uint8_t>(0x41 + (key - SDLK_a));
  }

  return 0x00;
}

}  // namespace irata2::frontend

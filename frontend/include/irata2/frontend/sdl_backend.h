#ifndef IRATA2_FRONTEND_SDL_BACKEND_H
#define IRATA2_FRONTEND_SDL_BACKEND_H

#include <cstdint>

#include <SDL.h>

#include "irata2/sim/io/vgc_backend.h"

namespace irata2::frontend {

class SdlBackend final : public sim::io::VgcBackend {
 public:
  explicit SdlBackend(SDL_Renderer* renderer);

  void clear(uint8_t intensity) override;
  void draw_point(uint8_t x, uint8_t y, uint8_t intensity) override;
  void draw_line(uint8_t x0,
                 uint8_t y0,
                 uint8_t x1,
                 uint8_t y1,
                 uint8_t intensity) override;
  void present() override;

 private:
  SDL_Renderer* renderer_;

  void SetColor(uint8_t intensity);
};

}  // namespace irata2::frontend

#endif  // IRATA2_FRONTEND_SDL_BACKEND_H

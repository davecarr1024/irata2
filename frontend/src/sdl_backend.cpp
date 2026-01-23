#include "irata2/frontend/sdl_backend.h"

namespace irata2::frontend {

SdlBackend::SdlBackend(SDL_Renderer* renderer) : renderer_(renderer) {}

void SdlBackend::clear(uint8_t intensity) {
  SetColor(intensity);
  SDL_RenderClear(renderer_);
}

void SdlBackend::draw_point(uint8_t x, uint8_t y, uint8_t intensity) {
  SetColor(intensity);
  SDL_RenderDrawPoint(renderer_, static_cast<int>(x), static_cast<int>(y));
}

void SdlBackend::draw_line(uint8_t x0,
                           uint8_t y0,
                           uint8_t x1,
                           uint8_t y1,
                           uint8_t intensity) {
  SetColor(intensity);
  SDL_RenderDrawLine(renderer_,
                     static_cast<int>(x0),
                     static_cast<int>(y0),
                     static_cast<int>(x1),
                     static_cast<int>(y1));
}

void SdlBackend::present() {
  SDL_RenderPresent(renderer_);
}

void SdlBackend::SetColor(uint8_t intensity) {
  const uint8_t level = intensity & 0x03;
  uint8_t green = 0;
  switch (level) {
    case 0:
      green = 0;
      break;
    case 1:
      green = 64;
      break;
    case 2:
      green = 128;
      break;
    case 3:
      green = 255;
      break;
    default:
      green = 0;
      break;
  }
  SDL_SetRenderDrawColor(renderer_, 0, green, 0, 255);
}

}  // namespace irata2::frontend

// SDLController.cpp

#include "svc/SDLController.hpp"
#include "logs.hpp"
#include <SDL2/SDL.h>

#define SDL_INIT_SUCCESS 0

namespace svc {
SDLController::SDLController() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != SDL_INIT_SUCCESS) {
    LOG_THROW(std::runtime_error, SDL_GetError());
  }
}

SDLController::~SDLController() {
  SDL_Quit();
}
} // namespace svc

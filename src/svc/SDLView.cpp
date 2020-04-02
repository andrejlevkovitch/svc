// SDLView.cpp

#include "svc/SDLView.hpp"
#include <SDL2/SDL.h>
#include <logs.hpp>

#define GL_MAJOR_3 3
#define GL_MINOR_0 0

namespace svc {
class SDLViewImp {
public:
  SDLViewImp(std::string_view title, int x, int y, int width, int height) {
    window_ = SDL_CreateWindow(std::string{title}.c_str(),
                               x,
                               y,
                               width,
                               height,
                               SDL_WINDOW_OPENGL);

    if (window_ == nullptr) {
      goto Exeption;
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (glContext_ == nullptr) {
      goto Exeption;
    }

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_ES) != 0 ||
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_MAJOR_3) != 0 ||
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_MINOR_0) != 0) {
      goto Exeption;
    }

    // and check attributs
    int glContextProfile;
    int glMajorVersion;
    int glMinorVersion;
    if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &glContextProfile) !=
            0 ||
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glMajorVersion) !=
            0 ||
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glMinorVersion) !=
            0) {
      goto Exeption;
    }

    if (glContextProfile != SDL_GL_CONTEXT_PROFILE_ES ||
        glMajorVersion != GL_MAJOR_3 || glMinorVersion != GL_MINOR_0) {
      SDL_SetError("couldn't set OpenGL ES 3.0");
      goto Exeption;
    }

    return;

  Exeption:
    SDL_DestroyWindow(window_);
    SDL_GL_DeleteContext(glContext_);
    LOG_THROW(std::runtime_error, SDL_GetError());
  }

  ~SDLViewImp() {
    SDL_DestroyWindow(window_);
    SDL_GL_DeleteContext(glContext_);
  }

  Size getContextSize() const noexcept {
    int width;
    int height;
    SDL_GL_GetDrawableSize(window_, &width, &height);

    return Size{float(width), float(height)};
  }

  void swapGLContext() noexcept {
    SDL_GL_SwapWindow(window_);
  }

  void makeCurrent() {
    if (SDL_GL_MakeCurrent(window_, glContext_) != 0) {
      LOG_THROW(std::runtime_error, SDL_GetError());
    }
  }

private:
  SDL_Window *  window_;
  SDL_GLContext glContext_;
};

SDLView::SDLView(std::string_view title, Point screenPos, Size windowSize)
    : imp_{new SDLViewImp{title,
                          int(screenPos.x()),
                          int(screenPos.y()),
                          int(windowSize.width()),
                          int(windowSize.height())}} {
}

SDLView::SDLView(std::string_view title, Size windowSize)
    : imp_{new SDLViewImp{title,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED,
                          int(windowSize.width()),
                          int(windowSize.height())}} {
}

SDLView::~SDLView() {
  delete imp_;
}

Size SDLView::getContextSize() const noexcept {
  return imp_->getContextSize();
}

void SDLView::makeCurrent() {
  imp_->makeCurrent();
}

void SDLView::render(AbstractVisitor *renderer) {
  this->accept(renderer);

  imp_->swapGLContext();
}
} // namespace svc

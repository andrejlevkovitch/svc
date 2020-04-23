// misc.hpp
/**\file contains auxilary functions, defines and macroses for OpenGL
 */

#include "logs.hpp"
#include <GLES3/gl3.h>
#include <string>

#define OUTPUT

#define GL_THROW_IF_ERROR()                                                    \
  if (GLenum status = glGetError(); status != GL_NO_ERROR) {                   \
    std::string error;                                                         \
    switch (status) {                                                          \
    case GL_INVALID_ENUM:                                                      \
      error = "gl invalid enum";                                               \
      break;                                                                   \
    case GL_INVALID_VALUE:                                                     \
      error = "gl invalid value";                                              \
      break;                                                                   \
    case GL_INVALID_OPERATION:                                                 \
      error = "gl invalid operation";                                          \
      break;                                                                   \
    case GL_INVALID_FRAMEBUFFER_OPERATION:                                     \
      error = "gl invalid framebuffer operation";                              \
      break;                                                                   \
    case GL_OUT_OF_MEMORY:                                                     \
      error = "gl out of memory";                                              \
      break;                                                                   \
    default:                                                                   \
      error = "unkhnown error";                                                \
      break;                                                                   \
    }                                                                          \
  }

namespace gl {
using GLShader         = GLuint;
using GLVertexShader   = GLuint;
using GLFragmentShader = GLuint;
using GLProgram        = GLuint;

void compileShader(const std::string &shaderSource,
                   OUTPUT GLShader    shader) noexcept(false);

void linkProgram(GLVertexShader   vertexShader,
                 GLFragmentShader fragmentShader,
                 OUTPUT GLProgram program) noexcept(false);

std::string getShaderInfoLog(GLShader shader) noexcept;

std::string getProgramInfoLog(GLProgram program) noexcept;
} // namespace gl

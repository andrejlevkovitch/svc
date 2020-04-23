// misc.cpp

#include "gl/misc.hpp"

namespace gl {
void compileShader(const std::string &shaderSource,
                   OUTPUT GLShader    shader) noexcept(false) {
  const char *sourceString = shaderSource.c_str();
  glShaderSource(shader, 1, &sourceString, nullptr);
  glCompileShader(shader);
  GLint compileStatus;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus != GL_TRUE) {
    LOG_THROW(std::runtime_error, getShaderInfoLog(shader));
  }
}

void linkProgram(GLVertexShader   vertexShader,
                 GLFragmentShader fragmentShader,
                 OUTPUT GLProgram program) noexcept(false) {
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  GLint linkStatus;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus != GL_TRUE) {
    LOG_THROW(std::runtime_error, getProgramInfoLog(program));
  }
  glValidateProgram(program);
  GLint validateStatus;
  glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
  if (validateStatus != GL_TRUE) {
    LOG_THROW(std::runtime_error, getProgramInfoLog(program));
  }
}

std::string getShaderInfoLog(GLShader shader) noexcept {
  GLint logLength = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
  std::string log;
  log.resize(logLength);
  glGetShaderInfoLog(shader, logLength, nullptr, log.data());
  return log;
}

std::string getProgramInfoLog(GLProgram program) noexcept {
  GLint logLength = 0;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
  std::string log;
  log.resize(logLength);
  glGetProgramInfoLog(program, logLength, nullptr, log.data());
  return log;
}
} // namespace gl

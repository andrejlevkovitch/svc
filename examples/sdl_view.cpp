// test_sdl.cpp

#include "gl/misc.hpp"
#include "logs.hpp"
#include "svc/AbstractItem.hpp"
#include "svc/SDLController.hpp"
#include "svc/SDLView.hpp"
#include "svc/Scene.hpp"
#include <GLES3/gl3.h>
#include <array>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <chrono>
#include <thread>

class SimpleItem;

namespace svc {
class AbstractVisitor {
public:
  virtual ~AbstractVisitor()           = default;
  virtual void visit(SimpleItem *itmp) = 0;
};
} // namespace svc

class SimpleItem final : public svc::AbstractItem {
public:
  svc::Box getBoundingBox() const noexcept override {
    return svc::Box{{-5, -5}, {5, 5}};
  }

  void accept(svc::AbstractVisitor *visitor) override {
    visitor->visit(this);
  }
};

const char vertexShaderSource[] = R"(
#version 300 es
uniform vec2 u_contextSize;
uniform mat3 u_viewMat;
uniform mat3 u_itemMat;

layout(location = 0) in vec2 a_point;
layout(location = 1) in vec4 a_color;

out vec4 v_color;

void main(void) {
  mat3 normMat = mat3(
    2.0 / u_contextSize.x,  0,                      0,
    0,                     -2.0 / u_contextSize.y,  0,
   -1,                      1,                      1
  );

  vec3 scenePos = (inverse(u_viewMat) * u_itemMat) * vec3(a_point, 1);
  vec3 normPos = normMat * scenePos;

  gl_Position = vec4(normPos, 1);
  v_color = a_color;
}
)";

const char fragmentShaderSource[] = R"(
#version 300 es
precision mediump float;

in vec4 v_color;

layout(location = 0) out vec4 o_fragColor;

void main(void) {
  o_fragColor = v_color;
}
)";

class GLRenderer final : public svc::AbstractVisitor {
  using VertexShader   = GLuint;
  using FragmentShader = GLuint;
  using Program        = GLuint;

public:
  GLRenderer() {
    program_        = glCreateProgram();
    vertexShader_   = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader_ = glCreateShader(GL_FRAGMENT_SHADER);
    GL_THROW_IF_ERROR();

    gl::compileShader(vertexShaderSource, vertexShader_);
    gl::compileShader(fragmentShaderSource, fragmentShader_);

    // at last link gl program
    gl::linkProgram(vertexShader_, fragmentShader_, program_);

    glUseProgram(program_);
    GL_THROW_IF_ERROR();
  }

  ~GLRenderer() {
    glDeleteShader(vertexShader_);
    glDeleteShader(fragmentShader_);
    glDeleteProgram(program_);
  }

  void prepare(const svc::SDLView &view) const noexcept {
    svc::Size   contextSize = view.getContextSize();
    svc::Matrix viewMat     = view.getSceneTransformMatrix();

    glViewport(0, 0, contextSize.width(), contextSize.height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform2fv(0, 1, contextSize.a);
    glUniformMatrix3fv(1, 1, true, viewMat.a[0]);
  }

  void visit(SimpleItem *item) override {
    svc::Box    itemBox = item->getBoundingBox();
    svc::Matrix itemMat = item->getSceneMatrix();

    svc::Ring itemRing;
    boost::geometry::convert(itemBox, itemRing);
    std::array<ushort, 4> itemTriangles{1,
                                        0,
                                        2,
                                        3}; // becuause it is simple rect
    std::array<float, 4>  color{1.0, 1.0, 1.0, 1.0};

    glUniformMatrix3fv(2, 1, true, itemMat.a[0]);

    glVertexAttrib4fv(1, color.data());
    glDisableVertexAttribArray(1);

    GLuint vbo{};
    GLuint ebo{};
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER,
                 itemRing.size() * sizeof(decltype(itemRing)::value_type),
                 itemRing.data(),
                 GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 itemTriangles.size() *
                     sizeof(decltype(itemTriangles)::value_type),
                 itemTriangles.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_TRIANGLE_STRIP,
                   itemTriangles.size(),
                   GL_UNSIGNED_SHORT,
                   nullptr);

    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
  }

private:
  VertexShader   vertexShader_;
  FragmentShader fragmentShader_;
  Program        program_;
};

int main() {
  svc::SDLController controller;

  svc::SDLView view{"hello", svc::Size{500, 500}};
  view.makeCurrent(); // not required, because we use only on View

  svc::ScenePtr scene = std::make_shared<svc::Scene>();
  svc::ItemPtr  item1 = std::make_shared<SimpleItem>();
  item1->setScenePos(svc::Point{300, 300});
  item1->setSceneRotation(TO_RAD(30));
  scene->appendItem(item1);
  view.setScene(scene);
  // view.setSceneRect(
  //    svc::Rect{svc::Point{200, 200}, svc::Size{200, 200}, TO_RAD(30)});

  GLRenderer renderer;
  renderer.prepare(view);
  view.render(&renderer);

  std::this_thread::sleep_for(std::chrono::milliseconds{2000});

  return EXIT_SUCCESS;
}

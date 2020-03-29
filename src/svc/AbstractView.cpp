// AbstractView.cpp

#include "svc/AbstractView.hpp"
#include <boost/qvm/map_vec_mat.hpp>
#include <boost/qvm/swizzle.hpp>
#include <svc/AbstractItem.hpp>
#include <svc/Scene.hpp>

namespace svc {
class AbstractViewImp {
public:
  AbstractViewImp() noexcept {
    matrix_ = bq::translation_mat(svc::Point{0, 0});
  }

  void setMatrix(Matrix mat) noexcept {
    matrix_ = std::move(mat);
  }

  Matrix getMatrix() const noexcept {
    return matrix_;
  }

  inline void moveOn(Point diff) noexcept {
    matrix_ *= bq::translation_mat(diff);
  }

  inline void rotate(float angle, Point anchor) noexcept {
    Matrix translationMat = bq::translation_mat(anchor);

    Matrix rotationMat = translationMat;
    rotationMat *= bq::rotz_mat<3>(angle);
    rotationMat *= bq::inverse(translationMat);

    matrix_ *= rotationMat;
  }

  inline float getRotation() const noexcept {
    float angle = svc::getRotation(matrix_);
    return angle;
  }

  inline void scale(ScaleFactors factors, Point anchor) noexcept {
    auto [xFactor, yFactor] = factors;

    Matrix translationMat = bq::translation_mat(anchor);

    Matrix scaleMatrix = translationMat;
    scaleMatrix *= bq::diag_mat(Vector{xFactor, yFactor, 1});
    scaleMatrix *= bq::inverse(translationMat);

    matrix_ *= scaleMatrix;
  }

  inline ScaleFactors getScaleFactors() const noexcept {
    return svc::getScaleFactors(matrix_);
  }

  /**\brief convert View point to Scene point
   */
  inline Point map(Point viewPoint) const noexcept {
    Vector retval = matrix_ * bq::XY1(viewPoint);
    return bq::XY(retval);
  }

private:
  /**\brief matrix which map View koordinates to Scene Koordinates
   */
  Matrix matrix_;
};

AbstractView::AbstractView() noexcept
    : imp_{new AbstractViewImp} {
}

AbstractView::~AbstractView() noexcept {
  delete imp_;

#ifndef NDEBUG
  imp_ = (AbstractViewImp *)0xdeadbeef;
#endif
}

void AbstractView::setScene(ScenePtr scene) noexcept {
  scene_ = scene;
}

ScenePtr AbstractView::getScene() const noexcept {
  return scene_;
}

void AbstractView::setSceneRect(Rect sceneRect) noexcept {
  Matrix rectMatrix = sceneRect.getMatrix();

  // so, we get matrix, but it is without scale. Scale is ratio sizes of rect
  // and view and scene
  Size rectSize = sceneRect.size();
  Size viewSize = this->size();

  float xFactor = rectSize.width() / viewSize.width();
  float yFactor = rectSize.height() / viewSize.height();

  Matrix scaleMatrix = bq::diag_mat(Vector{xFactor, yFactor, 1});
  rectMatrix *= scaleMatrix;

  imp_->setMatrix(std::move(rectMatrix));
}

Rect AbstractView::getSceneRect() const noexcept {
  Size viewSize = this->size();

  Point minCorner         = imp_->map(Point{0, 0});
  float angle             = imp_->getRotation();
  auto [xFactor, yFactor] = imp_->getScaleFactors();

  Size rectSize = {viewSize.width() * xFactor, viewSize.height() * yFactor};

  return Rect{minCorner, rectSize, angle};
}

void AbstractView::rotateSceneRect(float angle, svc::Point anchor) noexcept {
  imp_->rotate(angle, anchor);
}

void AbstractView::moveSceneRect(svc::Point vec) noexcept {
  imp_->moveOn(vec);
}

void AbstractView::scaleSceneRect(ScaleFactors factors, Point anchor) noexcept {
  imp_->scale(factors, anchor);
}

Matrix AbstractView::getSceneTransformMatrix() const noexcept {
  return imp_->getMatrix();
}

void AbstractView::setSceneTransformMatrix(Matrix mat) noexcept {
  imp_->setMatrix(std::move(mat));
}

Point AbstractView::mapToScene(Point viewPoint) const noexcept {
  return imp_->map(viewPoint);
}

void AbstractView::accept(AbstractVisitor *visitor) {
  if (scene_) {
    ItemList list = scene_->query(this->getSceneRect());
    std::for_each(list.begin(), list.end(), [visitor](ItemPtr &item) {
      item->accept(visitor);
    });
  }
}
} // namespace svc

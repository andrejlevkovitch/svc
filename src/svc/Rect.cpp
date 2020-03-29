// Rect.cpp

#include "svc/base_geometry_types.hpp"
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>
#include <boost/qvm/map_mat_vec.hpp>
#include <boost/qvm/map_vec_mat.hpp>

namespace svc {
Rect::Rect(Point minCorner, Size size, float angle, Point anchor)
    : matrix_{}
    , size_{size} {
  matrix_ = bq::translation_mat(minCorner);

  // and rotate
  Matrix translationMat = bq::translation_mat(anchor);

  Matrix rotationMat = translationMat;
  rotationMat *= bq::rotz_mat<3>(angle);
  rotationMat *= bq::inverse(translationMat);

  matrix_ *= rotationMat;
}

Rect::Rect(Box box) {
  svc::Point diag = box.max_corner() - box.min_corner();
  size_           = svc::Size{diag.x(), diag.y()};

  matrix_ = bq::translation_mat(box.min_corner());
}

void Rect::setMinCorner(Point minCorner) noexcept {
  bq::translation(matrix_) = minCorner;
}

Point Rect::getMinCorner() const noexcept {
  Point minCorner = bq::translation(matrix_);
  return minCorner;
}

void Rect::setSize(Size size) noexcept {
  size_ = size;
}

Size Rect::size() const noexcept {
  return size_;
}

void Rect::rotate(float angle, Point anchor) noexcept {
  Matrix translationMat = bq::translation_mat(anchor);

  Matrix rotationMat = translationMat;
  rotationMat *= bq::rotz_mat<3>(angle);
  rotationMat *= bq::inverse(translationMat);

  matrix_ *= rotationMat;
}

void Rect::setRotation(float angle, Point anchor) noexcept {
  Matrix translationMat = bq::translation_mat(anchor);

  Matrix resultMat = bq::translation_mat(bq::translation(matrix_));
  resultMat *= translationMat;
  resultMat *= bq::rotz_mat<3>(angle);
  resultMat *= bq::inverse(translationMat);

  matrix_ = std::move(resultMat);
}

float Rect::getRotation() const noexcept {
  float angle = svc::getRotation(matrix_);
  return NORM_RADIANS(angle);
}

void Rect::setMatrix(Matrix mat) noexcept {
  matrix_ = std::move(mat);
}

Matrix Rect::getMatrix() const noexcept {
  return matrix_;
}

void Rect::moveOn(Point diff) noexcept {
  matrix_ *= bq::translation_mat(diff);
}

Rect::operator Ring() const noexcept {
  Box box{{0, 0}, Point(this->size())};

  Ring baseRing;
  bg::convert(box, baseRing);

  Ring retval;
  bg::transform(
      baseRing,
      retval,
      bg::strategy::transform::matrix_transformer<float, 2, 2>{matrix_});

  return retval;
}
} // namespace svc

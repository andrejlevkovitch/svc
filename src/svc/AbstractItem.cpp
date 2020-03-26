// AbstractItem.cpp

#include "svc/AbstractItem.hpp"
#include "asserts.hpp"
#include "logs.hpp"
#include "svc/Scene.hpp"
#include <boost/qvm/map_mat_vec.hpp>
#include <boost/qvm/map_vec_mat.hpp>
#include <boost/qvm/mat_operations.hpp>
#include <boost/qvm/swizzle.hpp>

namespace svc {
std::pair<float, float> getScale(const Matrix &mat) {
  std::pair<float, float> xVec{mat.a[0][0], mat.a[1][0]};
  std::pair<float, float> yVec{mat.a[0][1], mat.a[1][1]};

  return {std::sqrt(std::pow(xVec.first, 2) + std::pow(xVec.second, 2)),
          std::sqrt(std::pow(yVec.first, 2) + std::pow(yVec.second, 2))};
}

float getRotation(const Matrix &mat) {
  auto [xFactor, yFactor] = getScale(mat);
  float sinAngl           = mat.a[0][1] / yFactor;
  float cosAngl           = mat.a[0][0] / xFactor;
  return std::atan2(-sinAngl, cosAngl);
}

using ItemPtr  = ItemPtr;
using Children = AbstractItem::Children;

class AbstractItemImp {
public:
  AbstractItemImp() noexcept
      : matrix_{} {
    matrix_ = bq::diag_mat(Vector{1, 1, 1});
  }

  ~AbstractItemImp() noexcept {
  }

  inline Point getPos() const noexcept {
    Vector pos{0, 0, 1};
    pos = matrix_ * pos;
    return bq::XY(pos);
  }

  inline void moveOn(Point diff) noexcept {
    bq::translation(matrix_) += diff;
  }

  inline Matrix getMatrix() const noexcept {
    return matrix_;
  }

  inline void setMatrix(Matrix matrix) noexcept {
    matrix_ = std::move(matrix);
  }

  inline float getRotation(Point anchor) const noexcept {
    Matrix mat         = matrix_;
    Matrix translation = bq::translation_mat(anchor);
    mat *= translation;
    return svc::getRotation(mat);
  }

  inline void rotate(float angle, Point anchor) noexcept {
    Matrix translationMat = bq::translation_mat(anchor);
    Matrix rotationMat    = bq::rotz_mat<3>(angle);

    Matrix newMat = bq::diag_mat(Vector{1, 1, 1});
    newMat *= translationMat;
    newMat *= rotationMat;
    newMat *= bq::inverse(translationMat);

    matrix_ *= newMat;
  }

  inline void setRotation(float angle, Point anchor) noexcept {
    Matrix translationMat = bq::translation_mat(anchor);
    Matrix rotationMat    = bq::rotz_mat<3>(angle);

    Matrix newMat = bq::diag_mat(Vector{1, 1, 1});
    newMat *= translationMat;
    newMat *= rotationMat;
    newMat *= bq::inverse(translationMat);

    Point pos = this->getPos();
    matrix_   = std::move(newMat);
    this->moveOn(pos);
  }

private:
  /**\brief store information relatively to parent (if Item don't has any parent
   * then the information is relative to Scene)
   */
  Matrix matrix_;
};

AbstractItem::AbstractItem() noexcept
    : imp_{new AbstractItemImp{}}
    , scene_{nullptr} {
}

AbstractItem::~AbstractItem() noexcept {
  Children children = this->children_;
  std::for_each(
      children.begin(),
      children.end(),
      std::bind(&AbstractItem::removeChild, this, std::placeholders::_1));

  delete imp_;

#ifndef NDEBUG
  children_.clear();
  imp_    = nullptr;
  scene_  = nullptr;
  parent_ = nullptr;
#endif
}

Scene *AbstractItem::getScene() const noexcept {
  return scene_;
}

Point AbstractItem::getPos() const noexcept {
  return imp_->getPos();
}

Point AbstractItem::getScenePos() const noexcept {
  Point pos = this->getPos();

  if (this->parent_) {
    Matrix      parentMatrix = this->parent_->getSceneMatrix();
    svc::Vector vec          = parentMatrix * bq::XY1(pos);
    pos                      = bq::XY(vec);
  }

  return pos;
}

void AbstractItem::moveOn(Point diff) noexcept {
  imp_->moveOn(diff);

  // TODO notificate Scene about changing position of Item
}

void AbstractItem::setPos(Point pos) noexcept {
  Point currentPos = imp_->getPos();
  Point diff       = pos - currentPos;
  imp_->moveOn(diff);
}

void AbstractItem::setScenePos(Point scenePos) noexcept {
  Point currentPos = this->getScenePos();
  Point diff       = scenePos - currentPos;
  imp_->moveOn(diff);
}

AbstractItem *AbstractItem::getParent() const noexcept {
  return parent_;
}

Children AbstractItem::getChildren() const noexcept {
  return children_;
}

void AbstractItem::appendChild(ItemPtr &child) {
  if (child.get() == nullptr) {
    LOG_THROW(std::runtime_error, "can't append invalid child");
  }

  if (AbstractItem *childParent = child->getParent()) {
    childParent->removeChild(child);
  }

  // before append to childs we must change matrix of child for save its Scene
  // position
  {
    Matrix childMatrix    = child->imp_->getMatrix();
    Matrix parentMatrix   = this->getSceneMatrix();
    Matrix newChildMatrix = bq::inverse(parentMatrix) * childMatrix;
    child->imp_->setMatrix(newChildMatrix);
  }

  child->parent_ = this;
  this->children_.emplace_back(child);

  if (Scene *childScene = child->getScene(); scene_ && (childScene != scene_)) {
    scene_->appendItem(child);
  } else if (scene_ == nullptr && childScene) {
    childScene->removeItem(child);
  }
}

void AbstractItem::removeChild(ItemPtr &child) {
  if (child.get() == nullptr || this != child->parent_) {
    LOG_THROW(std::runtime_error, "child has different parent");
  }

  // at first we need change child, especially its matrix, because if the Item
  // will be set to another parent (or set to Scene), we Item must save its
  // Scene position
  Matrix childSceneMatrix = child->getSceneMatrix();
  child->parent_          = nullptr;

  auto forRemove = std::find(children_.begin(), children_.end(), child);

  DEBBUG_ASSERT(forRemove != children_.end(), "child not found");

  children_.erase(forRemove);

  child->imp_->setMatrix(std::move(childSceneMatrix));

  // XXX NOTE: use child Scene, because if child already removed from Scene,
  // then its Scene was set to nullptr. If you use parent Scene you can get
  // recursive call
  if (Scene *childScene = child->getScene()) {
    childScene->removeItem(child);
  }
}

Matrix AbstractItem::getMatrix() const noexcept {
  return imp_->getMatrix();
}

Matrix AbstractItem::getSceneMatrix() const noexcept {
  Matrix matrix = imp_->getMatrix();

  if (this->parent_) {
    Matrix parentMatrix = this->parent_->getSceneMatrix();
    matrix              = parentMatrix * matrix;
  }

  return matrix;
}

float AbstractItem::getRotation(Point anchor) const noexcept {
  float angle = imp_->getRotation(anchor);
  return NORM_RADIANS(angle);
}

float AbstractItem::getSceneRotation(Point anchor) const noexcept {
  Matrix mat         = this->getSceneMatrix();
  Matrix translation = bq::translation_mat(anchor);
  mat *= translation;
  float angle = svc::getRotation(mat);
  return NORM_RADIANS(angle);
}

void AbstractItem::rotate(float angle, Point anchor) noexcept {
  imp_->rotate(angle, anchor);
}

void AbstractItem::setRotation(float angle, Point anchor) noexcept {
  imp_->setRotation(angle, anchor);
}

void AbstractItem::setSceneRotation(float angle, Point anchor) noexcept {
  Matrix translationMat = bq::translation_mat(anchor);
  Matrix rotationMat    = bq::rotz_mat<3>(angle);

  Matrix newMat = bq::diag_mat(Vector{1, 1, 1});
  newMat *= translationMat;
  newMat *= rotationMat;
  newMat *= bq::inverse(translationMat);

  if (this->parent_) {
    Matrix parentMatrix = this->parent_->getSceneMatrix();
    newMat *= bq::inverse(parentMatrix);
  }

  Point pos = this->getPos();
  imp_->setMatrix(std::move(newMat));
  this->moveOn(pos);
}

void AbstractItem::setScene(Scene *scene) noexcept {
  scene_ = scene;
}
} // namespace svc

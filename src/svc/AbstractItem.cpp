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

  inline void moveOn(Point diff) noexcept {
    matrix_ *= bq::translation_mat(diff);
  }

  inline void setPos(Point pos) noexcept {
    bq::translation(matrix_) = pos;
  }

  inline Point getPos() const noexcept {
    Vector pos{0, 0, 1};
    pos = matrix_ * pos;
    return bq::XY(pos);
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

    bq::translation(newMat) = bq::translation(matrix_);
    matrix_                 = std::move(newMat);
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
  std::for_each(children.begin(), children.end(), [this](ItemPtr &child) {
    this->removeChild(child.get());
  });

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
  Matrix      matrix = this->getSceneMatrix();
  svc::Vector vec{0, 0, 1};
  svc::Vector pos = matrix * vec;

  return bq::XY(pos);
}

void AbstractItem::moveOn(Point diff) noexcept {
  imp_->moveOn(diff);

  if (scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setPos(Point pos) noexcept {
  imp_->setPos(pos);

  if (scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setScenePos(Point scenePos) noexcept {
  // XXX because all information about position stores in koordinates relatively
  // to parent, we need transform the position from absolute koordinates to
  // relative
  if (parent_) {
    Matrix      sceneMatrix = this->parent_->getSceneMatrix();
    svc::Vector relativePos = bq::inverse(sceneMatrix) * bq::XY1(scenePos);
    imp_->setPos(bq::XY(relativePos));
  } else {
    imp_->setPos(scenePos);
  }

  if (scene_) {
    scene_->updateItemPosition(this);
  }
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
    childParent->removeChild(child.get());
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
    childScene->removeItem(child.get());
  }
}

void AbstractItem::removeChild(AbstractItem *child) {
  if (child == nullptr || this != child->parent_) {
    LOG_THROW(std::runtime_error, "child has different parent");
  }

  // at first we need change child, especially its matrix, because if the Item
  // will be set to another parent (or set to Scene), we Item must save its
  // Scene position
  Matrix childSceneMatrix = child->getSceneMatrix();
  child->parent_          = nullptr;

  auto forRemove = std::find_if(children_.begin(),
                                children_.end(),
                                [child](const ItemPtr &item) {
                                  if (child == item.get()) {
                                    return true;
                                  }
                                  return false;
                                });

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

  if (anchor != Point{0, 0} && scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setRotation(float angle, Point anchor) noexcept {
  imp_->setRotation(angle, anchor);

  if (anchor != Point{0, 0} && scene_) {
    scene_->updateItemPosition(this);
  }
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

  Matrix currentMat       = imp_->getMatrix();
  bq::translation(newMat) = bq::translation(currentMat);

  imp_->setMatrix(std::move(newMat));

  if (anchor != Point{0, 0} && scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setScene(Scene *scene) noexcept {
  scene_ = scene;
}
} // namespace svc

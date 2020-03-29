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
    Point pos = bq::translation(matrix_);
    return pos;
  }

  inline Matrix getMatrix() const noexcept {
    return matrix_;
  }

  inline void setMatrix(Matrix matrix) noexcept {
    matrix_ = std::move(matrix);
  }

  inline float getRotation() const noexcept {
    float angle = svc::getRotation(matrix_);
    return angle;
  }

  inline void rotate(float angle, Point anchor) noexcept {
    Matrix translationMat = bq::translation_mat(anchor);

    Matrix rotationMat = translationMat;
    rotationMat *= bq::rotz_mat<3>(angle);
    rotationMat *= bq::inverse(translationMat);

    matrix_ *= rotationMat;
  }

  inline void setRotation(float angle, Point anchor) noexcept {
    Matrix translationMat = bq::translation_mat(anchor);

    Matrix resultMat = bq::translation_mat(bq::translation(matrix_));
    resultMat *= translationMat;
    resultMat *= bq::rotz_mat<3>(angle);
    resultMat *= bq::inverse(translationMat);

    matrix_ = std::move(resultMat);
  }

private:
  /**\brief store information relatively to parent (if Item don't has any parent
   * then the information is relative to Scene)
   */
  Matrix matrix_;
};

AbstractItem::AbstractItem() noexcept
    : imp_{new AbstractItemImp{}}
    , scene_{nullptr}
    , parent_{nullptr} {
}

AbstractItem::~AbstractItem() noexcept {
  Children children = this->children_;
  std::for_each(children.begin(), children.end(), [this](ItemPtr &child) {
    this->removeChild(child.get());
  });

  delete imp_;

#ifndef NDEBUG
  children_.clear();
  imp_    = (AbstractItemImp *)0xdeadbeef;
  scene_  = (Scene *)0xdeadbeef;
  parent_ = (AbstractItem *)0xdeadbeef;
#endif
}

Scene *AbstractItem::getScene() const noexcept {
  return scene_;
}

Point AbstractItem::getPos() const noexcept {
  return imp_->getPos();
}

Point AbstractItem::getScenePos() const noexcept {
  Matrix     matrix = this->getSceneMatrix();
  svc::Point pos    = bq::translation(matrix);
  return pos;
}

void AbstractItem::moveOn(Point diff) {
  imp_->moveOn(diff);

  if (scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setPos(Point pos) {
  imp_->setPos(pos);

  if (scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setScenePos(Point scenePos) {
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

void AbstractItem::setMatrix(Matrix matrix) {
  imp_->setMatrix(std::move(matrix));

  if (scene_) {
    scene_->updateItemPosition(this);
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

float AbstractItem::getRotation() const noexcept {
  float angle = imp_->getRotation();
  return NORM_RADIANS(angle);
}

float AbstractItem::getSceneRotation() const noexcept {
  Matrix mat   = this->getSceneMatrix();
  float  angle = svc::getRotation(mat);
  return NORM_RADIANS(angle);
}

void AbstractItem::rotate(float angle, Point anchor) {
  imp_->rotate(angle, anchor);

  // XXX if anchor is default, then we not need update position, becuase
  // position of the Item didn't change
  if (anchor != Point{0, 0} && scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setRotation(float angle, Point anchor) {
  imp_->setRotation(angle, anchor);

  if (anchor != Point{0, 0} && scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setSceneRotation(float angle, Point anchor) {
  Matrix translationMat = bq::translation_mat(anchor);

  Matrix currentMat = imp_->getMatrix();

  Matrix resultMat = bq::translation_mat(bq::translation(currentMat));
  resultMat *= translationMat;
  resultMat *= bq::rotz_mat<3>(angle);
  resultMat *= bq::inverse(translationMat);

  if (this->parent_) {
    Matrix parentMatrix = this->parent_->getSceneMatrix();
    resultMat *= bq::inverse(parentMatrix);
  }

  imp_->setMatrix(std::move(resultMat));

  if (anchor != Point{0, 0} && scene_) {
    scene_->updateItemPosition(this);
  }
}

void AbstractItem::setScene(Scene *scene) noexcept {
  scene_ = scene;
}

bool AbstractItem::empty() const noexcept {
  return children_.empty();
}

size_t AbstractItem::count() const noexcept {
  return children_.size();
}
} // namespace svc

// AbstractItem.cpp

#include "svc/AbstractItem.hpp"
#include "asserts.hpp"
#include "svc/Scene.hpp"

namespace svc {
using ChildItemPtr = AbstractItem::ChildItemPtr;

AbstractItem::AbstractItem() noexcept
    : scene_{nullptr} {
}

Scene *AbstractItem::getScene() const noexcept {
  return scene_;
}

AbstractItem *AbstractItem::parent() const noexcept {
  return parent_;
}

const std::list<ChildItemPtr> &AbstractItem::children() const noexcept {
  return children_;
}

void AbstractItem::appendChild(ChildItemPtr child) noexcept {
  if (child.get() == nullptr) {
    return;
  }

  if (child->parent()) {
    child->parent_->removeChild(*child);
  }

  child->parent_ = this;
  this->children_.emplace_back(std::move(child));
}

void AbstractItem::removeChild(AbstractItem &child) noexcept {
  ASSERT(this != child.parent(),
         "removing from children Item have different parent");

  children_.remove_if([ptr = &child](const ChildItemPtr &item) {
    if (item.get() == ptr) {
      return true;
    }
    return false;
  });
}

void AbstractItem::setScene(Scene *scene) noexcept {
  scene_ = scene;
}
} // namespace svc

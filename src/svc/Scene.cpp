// Scene.cpp

#include "svc/Scene.hpp"
#include "logs.hpp"
#include "svc/AbstractItem.hpp"
#include "svc/base_geometry_types.hpp"
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/qvm/map_vec_mat.hpp>
#include <iterator>
#include <list>

#define MAX_NUMBER_VALUES_IN_NODE 16

namespace bg = boost::geometry;

using TranslateStrategy = bg::strategy::transform::translate_transformer<
    bg::traits::coordinate_type<svc::Point>::type,
    bg::traits::dimension<svc::Point>::value,
    bg::traits::dimension<svc::Point>::value>;

namespace svc {
class SceneImp {
  enum ValueTypes { BoxType, ItemType };
  using Value = std::tuple<Box, ItemPtr>;
  struct ValueComparator {
    bool operator()(const Value &first, const Value &second) const {
      if (std::get<ItemType>(first) == std::get<ItemType>(second)) {
        return true;
      }
      return false;
    }
  };

  using Index = bg::index::quadratic<MAX_NUMBER_VALUES_IN_NODE>;
  using RTree = bg::index::
      rtree<Value, Index, bg::index::indexable<Value>, ValueComparator>;

public:
  SceneImp() noexcept
      : tree_{} {
  }

  void appendItem(ItemPtr &item) {
    // all Items we store by its bounding boxes, BUT! bounding boxes defined in
    // Items koordinates (without rotating), so we need translate it to Scene
    // koordinates
    Box   itemBoundingBox = item->getBoundingBox();
    Point itemScenePos    = item->getScenePos();

    Box sceneBoundingBox;
    bg::transform(itemBoundingBox,
                  sceneBoundingBox,
                  TranslateStrategy{itemScenePos.x(), itemScenePos.y()});

    tree_.insert(Value{sceneBoundingBox, item});
  }

  void removeItem(ItemPtr &item) {
    // XXX we not need calculate bounding box, because we compare Values only by
    // ItemPtr
    size_t count = tree_.remove(Value(Box{}, item));
    if (count == 0) {
      LOG_THROW(std::runtime_error, "item not removed");
    }
  }

  size_t count() const noexcept {
    return tree_.size();
  }

  bool empty() const noexcept {
    return tree_.empty();
  }

  void clear() noexcept {
    tree_.clear();
  }

  Box bounds() const noexcept {
    auto box = tree_.bounds();
    return Box{{bg::get<0>(box.min_corner()), bg::get<1>(box.min_corner())},
               {bg::get<0>(box.max_corner()), bg::get<1>(box.max_corner())}};
  }

  /**\brief Forward Iterator
   */
  class ConstIterator {
    friend SceneImp;

  public:
    ConstIterator &operator++() noexcept {
      ++imp_;
      return *this;
    }

    const ItemPtr &operator*() const noexcept {
      return std::get<ItemType>(*imp_);
    }

    bool operator!=(const ConstIterator &second) const noexcept {
      return this->imp_ != second.imp_;
    }

    bool operator==(const ConstIterator &second) const noexcept {
      return this->imp_ == second.imp_;
    }

  private:
    ConstIterator(RTree::const_iterator iter) noexcept
        : imp_{std::move(iter)} {
    }

  private:
    RTree::const_iterator imp_;
  };

  ConstIterator begin() const noexcept {
    return ConstIterator{tree_.begin()};
  }

  ConstIterator end() const noexcept {
    return ConstIterator{tree_.end()};
  }

private:
  RTree tree_;
};

Scene::Scene() noexcept
    : imp_{new SceneImp{}} {
}

Scene::~Scene() noexcept {
  std::for_each(imp_->begin(), imp_->end(), [](const ItemPtr &item) {
    item->setScene(nullptr);
  });

  delete imp_;
}

static void recursiveChildCall(std::function<void(ItemPtr &)> foo,
                               ItemPtr &                      item) {
  std::list<ItemPtr> children = item->getChildren();
  for_each(children.begin(), children.end(), foo);
}

void Scene::appendItem(ItemPtr &item) {
  if (item.get() == nullptr) {
    LOG_THROW(std::runtime_error, "can't append invalid item");
  }

  if (AbstractItem *parent = item->getParent();
      parent && parent->getScene() != this) {
    parent->removeChild(item); // also remove the item from another Scene
  } else if (Scene *itemScene = item->getScene();
             itemScene && itemScene != this) {
    itemScene->removeItem(item);
  }

  imp_->appendItem(item);
  item->setScene(this);

  recursiveChildCall(
      [this](ItemPtr &child) {
        this->imp_->appendItem(child);
        child->setScene(this);
      },
      item);
}

void Scene::removeItem(ItemPtr &item) {
  if (item.get() == nullptr) {
    LOG_THROW(std::runtime_error, "can't append invalid item");
  }

  // XXX before removing the Item from children we need set its scene as nullptr
  // for prevent don't call the function recursively
  item->setScene(nullptr);
  if (AbstractItem *parentItem = item->getParent()) {
    parentItem->removeChild(item);
  }

  imp_->removeItem(item);

  recursiveChildCall(
      [this](ItemPtr &child) {
        this->imp_->removeItem(child);
        child->setScene(nullptr);
      },
      item);
}

size_t Scene::count() const noexcept {
  return imp_->count();
}

bool Scene::empty() const noexcept {
  return imp_->empty();
}

void Scene::clear() noexcept {
  std::for_each(imp_->begin(), imp_->end(), [](const ItemPtr &item) {
    item->setScene(nullptr);
  });

  imp_->clear();
}

Box Scene::bounds() const noexcept {
  return imp_->bounds();
}
} // namespace svc

namespace std {
template <>
class iterator_traits<svc::SceneImp::ConstIterator> {
  using difference_type   = std::ptrdiff_t;
  using value_type        = svc::ItemPtr;
  using reference_type    = const svc::ItemPtr &;
  using iterator_category = std::forward_iterator_tag;
};
} // namespace std

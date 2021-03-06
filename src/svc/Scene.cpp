// Scene.cpp

#include "svc/Scene.hpp"
#include "asserts.hpp"
#include "logs.hpp"
#include "svc/AbstractItem.hpp"
#include "svc/base_geometry_types.hpp"
#include <boost/function_output_iterator.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/qvm/map_vec_mat.hpp>
#include <iterator>
#include <list>

#ifndef NDEBUG
#  include <boost/geometry/algorithms/is_convex.hpp>
#  include <boost/geometry/algorithms/is_valid.hpp>
#endif

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

  void appendItem(ItemPtr item) {
    // all Items we store by its bounding boxes, BUT! bounding boxes defined in
    // Items koordinates (without rotating), so we need translate it to Scene
    // koordinates
    Box   itemBoundingBox = item->getBoundingBox();
    Point itemScenePos    = item->getScenePos();

    Box sceneBoundingBox;
    bg::transform(itemBoundingBox,
                  sceneBoundingBox,
                  TranslateStrategy{itemScenePos.x(), itemScenePos.y()});

    tree_.insert(Value{sceneBoundingBox, std::move(item)});
  }

  void removeItem(AbstractItem *item) {
    auto found =
        std::find_if(tree_.begin(), tree_.end(), [item](const Value &value) {
          if (std::get<ValueTypes::ItemType>(value).get() == item) {
            return true;
          }
          return false;
        });

    if (found == tree_.end()) {
      LOG_THROW(std::runtime_error, "item not found");
    }

    size_t count = tree_.remove(*found);
    if (count == 0) {
      LOG_THROW(std::runtime_error, "item not removed");
    }
  }

  void updateItemPosition(AbstractItem *item) {
    auto found =
        std::find_if(tree_.begin(), tree_.end(), [item](const Value &val) {
          if (std::get<ValueTypes::ItemType>(val).get() == item) {
            return true;
          }
          return false;
        });

    if (found == tree_.end()) {
      LOG_THROW(std::runtime_error, "item not found");
    }

    ItemPtr movingItem = std::get<ValueTypes::ItemType>(*found);
    size_t  count      = tree_.remove(*found);
    if (count == 0) {
      LOG_THROW(std::runtime_error, "item not removed");
    }

    this->appendItem(std::move(movingItem));
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

  ItemList query(Point pos) const noexcept {
    ItemList retval;

    auto back_inserter =
        boost::make_function_output_iterator([&retval](const Value &val) {
          retval.emplace_back(std::get<ValueTypes::ItemType>(val));
        });

    tree_.query(bg::index::covers(pos), back_inserter);

    return retval;
  }

  template <typename GeometryType,
            typename = typename std::enable_if<
                bg::is_areal<GeometryType>::value>::type>
  ItemList query(GeometryType                         geometry,
                 [[maybe_unused]] Scene::SpatialIndex index) const noexcept {
    ItemList retval;

    auto back_inserter =
        boost::make_function_output_iterator([&retval](const Value &val) {
          retval.emplace_back(std::get<ValueTypes::ItemType>(val));
        });

    if constexpr (std::is_same<GeometryType, Box>::value) {
      switch (index) {
      case Scene::SpatialIndex::Intersects:
        tree_.query(bg::index::intersects(geometry), back_inserter);
        break;
      case Scene::SpatialIndex::Within:
        tree_.query(bg::index::within(geometry), back_inserter);
        break;
      }
    } else {
      tree_.query(bg::index::intersects(geometry), back_inserter);
    }

    return retval;
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
                               AbstractItem *                 item) {
  std::list<ItemPtr> children = item->getChildren();
  for_each(children.begin(), children.end(), [foo](ItemPtr &child) {
    foo(child);
    recursiveChildCall(foo, child.get());
  });
}

void Scene::appendItem(ItemPtr &item) {
  if (item.get() == nullptr) {
    LOG_THROW(std::runtime_error, "can't append invalid item");
  }

  if (AbstractItem *parent = item->getParent();
      parent && parent->getScene() != this) {
    parent->removeChild(item.get()); // also remove the item from another Scene
  } else if (Scene *itemScene = item->getScene();
             itemScene && itemScene != this) {
    itemScene->removeItem(item.get());
  }

  imp_->appendItem(item);
  item->setScene(this);

  recursiveChildCall(
      [this](ItemPtr &child) {
        this->imp_->appendItem(child);
        child->setScene(this);
      },
      item.get());
}

void Scene::removeItem(AbstractItem *item) {
  if (item == nullptr) {
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
        this->imp_->removeItem(child.get());
        child->setScene(nullptr);
      },
      item);
}

void Scene::updateItemPosition(AbstractItem *item) {
  imp_->updateItemPosition(item);

  recursiveChildCall(
      [this](ItemPtr &child) {
        this->imp_->updateItemPosition(child.get());
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

ItemList Scene::query(Point pos) const noexcept {
  return imp_->query(pos);
}

ItemList Scene::query(Box box, SpatialIndex index) const noexcept {
  return imp_->query(box, index);
}

ItemList Scene::query(Ring ring, SpatialIndex index) const noexcept {
  DEBBUG_ASSERT(bg::is_valid(ring) && bg::is_convex(ring),
                "ring must be valid convex ring");

  return imp_->query(ring, index);
}

void Scene::accept(AbstractVisitor *visitor) {
  std::for_each(imp_->begin(), imp_->end(), [visitor](const ItemPtr &item) {
    if (item->getParent() == nullptr) { // only for main items
      item->accept(visitor);
    }
  });
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

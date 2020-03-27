// test_Scene.cpp

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

//

#include "svc/AbstractItem.hpp"
#include "svc/Scene.hpp"
#include "test_auxilary.hpp"
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/strategies/strategies.hpp>

class BasicItem final : public svc::AbstractItem {
public:
  svc::Box getBoundingBox() const noexcept override {
    return {{-5, -5}, {5, 5}};
  }

  void accept([[maybe_unused]] svc::AbstractVisitor *visitor) override {
  }
};

SCENARIO("test Scene", "[Scene]") {
  GIVEN("empty Scene") {
    svc::Scene scene;

    THEN("it must be empty") {
      CHECK(scene.empty());
      CHECK(scene.count() == 0);
    }

    THEN("bounds must return invalid box") {
      CHECK_NOTHROW(scene.bounds());
    }

    WHEN("we try remove not existing Item") {
      THEN("it must produce execption") {
        CHECK_THROWS(scene.removeItem(nullptr));

        svc::ItemPtr itemWithoutScene = std::make_shared<BasicItem>();
        CHECK_THROWS(scene.removeItem(itemWithoutScene.get()));
      }
    }

    WHEN("try add invalid Item") {
      THEN("throw exception") {
        svc::ItemPtr invalidItem;
        CHECK_THROWS(scene.appendItem(invalidItem));
      }
    }
  }

  GIVEN("Scene with one Item") {
    std::shared_ptr<svc::Scene> scene = std::make_shared<svc::Scene>();

    svc::ItemPtr item         = std::make_shared<BasicItem>();
    svc::Point   initialPoint = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    float        initialAngle = ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    item->setScenePos(initialPoint);
    item->setSceneRotation(initialAngle);

    scene->appendItem(item);

    THEN("scene can not be empty") {
      CHECK(scene->empty() == false);
      CHECK(scene->count() == 1);
    }

    THEN("the Item must be associated with the Scene") {
      CHECK(item->getScene() == scene.get());
    }

    THEN("position and angle of the Item must be same as before") {
      svc::Point pos   = item->getScenePos();
      float      angle = item->getSceneRotation();

      CHECK_ANGLES_EQUAL(angle, initialAngle);
      CHECK_POINTS_EQUAL(pos, initialPoint);
    }

    THEN("Minimal Bounding Box for Items must be same as bounding Box of "
         "singular Item") {
      svc::Box sceneBoundsBox  = scene->bounds();
      svc::Box itemBoundingBox = item->getBoundingBox();

      CHECK(Approx{bg::area(sceneBoundsBox)} == bg::area(itemBoundingBox));

      svc::Point scenePos = item->getScenePos();
      svc::Point minimalBoundingBoxCorner =
          scenePos + itemBoundingBox.min_corner();

      CHECK_POINTS_EQUAL(minimalBoundingBoxCorner, sceneBoundsBox.min_corner());
    }

    WHEN("remove Item") {
      scene->removeItem(item.get());

      THEN("now the Item don't associated with any Scene") {
        CHECK(item->getScene() == nullptr);
      }

      WHEN("try remove again") {
        THEN("produce error") {
          CHECK_THROWS(scene->removeItem(item.get()));
        }
      }
    }

    WHEN("clear Scene") {
      scene->clear();

      THEN("the Scene must be empty") {
        CHECK(scene->empty());
        CHECK(scene->count() == 0);
      }

      THEN("the Item must not be associated with any Scene") {
        CHECK(item->getScene() == nullptr);
      }

      THEN("Scene has invalid bounds Box") {
        CHECK_NOTHROW(scene->bounds());
      }
    }

    WHEN("remove Scene") {
      scene.reset();

      THEN("the Item must don't has associated Scene") {
        CHECK(item->getScene() == nullptr);
      }
    }
  }

  GIVEN("Scene with several Items without parent") {
    std::shared_ptr<svc::Scene> scene = std::make_shared<svc::Scene>();

    size_t itemCount = 10;
    for (size_t i = 0; i < itemCount; ++i) {
      svc::ItemPtr item         = std::make_shared<BasicItem>();
      svc::Point   initialPoint = POINT_GENERATOR(THIRD_LEVEL_GENERATOR);
      float        initialAngle = ANGLE_GENERATOR(THIRD_LEVEL_GENERATOR);
      item->setScenePos(initialPoint);
      item->setSceneRotation(initialAngle);

      scene->appendItem(item);
    }

    THEN("check count of Items in scene") {
      CHECK(scene->count() == itemCount);
    }

    WHEN("use clear") {
      scene->clear();
      THEN("all Item-s must be removed") {
        CHECK(scene->empty());
        CHECK(scene->count() == 0);
      }
    }
  }

  GIVEN("Scene and Item with child") {
    std::shared_ptr<svc::Scene> scene = std::make_shared<svc::Scene>();

    svc::ItemPtr parent = std::make_shared<BasicItem>();
    svc::ItemPtr child  = std::make_shared<BasicItem>();

    WHEN("append child to parent before add to Scene") {
      parent->appendChild(child);
      scene->appendItem(parent);

      THEN("Scene must has two Items") {
        CHECK(scene->count() == 2);
      }

      WHEN("remove child by Scene") {
        scene->removeItem(child.get());

        THEN("Scene must contains only one Item") {
          CHECK(scene->count() == 1);
        }

        THEN("parent must not cantains any children anymore") {
          CHECK(parent->getChildren().size() == 0);
        }

        THEN("child must not has any Scene") {
          CHECK(child->getScene() == nullptr);
        }
      }

      WHEN("remove child by parent") {
        parent->removeChild(child.get());

        THEN("Scene must contains only one Item") {
          CHECK(scene->count() == 1);
        }

        THEN("parent must not cantains any children anymore") {
          CHECK(parent->getChildren().size() == 0);
        }

        THEN("child must not has any Scene") {
          CHECK(child->getScene() == nullptr);
        }
      }

      WHEN("remove parent") {
        scene->removeItem(parent.get());

        THEN("Scene must be empty") {
          scene->empty();
        }

        THEN("parent and child not has any Scene") {
          CHECK(parent->getScene() == nullptr);
          CHECK(child->getScene() == nullptr);
        }

        THEN("parent still has own child") {
          CHECK(parent->getChildren().size() == 1);
          CHECK(parent->getChildren().front() == child);
        }
      }
    }

    WHEN("append child to parent and add to Scene only child") {
      parent->appendChild(child);
      scene->appendItem(child);

      THEN("Scene must contains only child") {
        CHECK(scene->count() == 1);

        CHECK(child->getScene() == scene.get());
        CHECK(parent->getScene() == nullptr);
      }

      THEN("parent don't has any children") {
        CHECK(parent->getChildren().empty());
        CHECK(child->getParent() == nullptr);
      }
    }

    WHEN("first append parent and next child to parent") {
      scene->appendItem(parent);
      parent->appendChild(child);

      THEN("scene must contains two elements") {
        CHECK(scene->count() == 2);
      }

      THEN("both (parent and child) must be associated with the Scene") {
        CHECK(parent->getScene() == scene.get());
        CHECK(child->getScene() == scene.get());
      }
    }

    WHEN("first append child to Scene add child to parent, which not "
         "associated with any Scene") {
      scene->appendItem(child);
      parent->appendChild(child);

      THEN("Scene must be empty") {
        CHECK(scene->empty());
      }

      THEN("child don't associated with any Scene") {
        CHECK(child->getScene() == nullptr);
      }
    }

    WHEN("first add child and parent to Scene and after add child to parent") {
      scene->appendItem(parent);
      scene->appendItem(child);
      parent->appendChild(child);

      THEN("scene must contains two elements") {
        CHECK(scene->count() == 2);
      }

      THEN("child and parent must be associated with the Scene") {
        CHECK(parent->getScene() == scene.get());
        CHECK(child->getScene() == scene.get());
      }
    }
  }

  GIVEN("Scene with several nested Item-s") {
    std::shared_ptr<svc::Scene> scene = std::make_shared<svc::Scene>();

    svc::ItemPtr first  = std::make_shared<BasicItem>();
    svc::ItemPtr second = std::make_shared<BasicItem>();
    svc::ItemPtr third  = std::make_shared<BasicItem>();
    svc::ItemPtr fourd  = std::make_shared<BasicItem>();

    first->appendChild(second);
    second->appendChild(third);
    third->appendChild(fourd);

    scene->appendItem(first);

    THEN("all Items must be on Scene") {
      CHECK(scene->count() == 4);

      CHECK(first->getScene() == scene.get());
      CHECK(second->getScene() == scene.get());
      CHECK(third->getScene() == scene.get());
      CHECK(fourd->getScene() == scene.get());
    }

    WHEN("remove root item") {
      scene->removeItem(first.get());

      THEN("Scene must be empty") {
        CHECK(scene->empty());
      }

      THEN("no one Item don't associated with the Scene") {
        CHECK(first->getScene() == nullptr);
        CHECK(second->getScene() == nullptr);
        CHECK(third->getScene() == nullptr);
        CHECK(fourd->getScene() == nullptr);
      }
    }
  }

  GIVEN("two Scene and Item-s for changing ownership") {
    std::shared_ptr<svc::Scene> scene1 = std::make_shared<svc::Scene>();
    std::shared_ptr<svc::Scene> scene2 = std::make_shared<svc::Scene>();

    svc::ItemPtr parent = std::make_shared<BasicItem>();
    svc::ItemPtr child  = std::make_shared<BasicItem>();

    WHEN("we append the Item from one Scene to other") {
      scene1->appendItem(parent);
      scene2->appendItem(parent);

      THEN("first Scene must be empty, but second Scene must has the Item") {
        CHECK(scene1->empty());

        CHECK(scene2->count() == 1);
      }

      THEN("Item must be associated with second Scene") {
        CHECK(parent->getScene() == scene2.get());
      }
    }

    GIVEN("two nested Items in first Scene") {
      parent->appendChild(child);
      scene1->appendItem(parent);

      WHEN("move parent to second Scene") {
        scene2->appendItem(parent);

        THEN(
            "first Scene must be empty, but second must has parent and child") {
          CHECK(scene1->empty());
          CHECK(scene2->count() == 2);
        }

        THEN("parent and child associated with second Scene") {
          CHECK(parent->getScene() == scene2.get());
          CHECK(child->getScene() == scene2.get());
        }

        THEN("child still has parent") {
          CHECK(child->getParent() == parent.get());
        }
      }

      WHEN("move child to second Scene") {
        scene2->appendItem(child);

        THEN("first Scene still has one Item, second Scene also has one Item") {
          CHECK(scene1->count() == 1);
          CHECK(scene2->count() == 1);
        }

        THEN("check right association") {
          CHECK(parent->getScene() == scene1.get());
          CHECK(child->getScene() == scene2.get());
        }

        THEN("parent don't has any children") {
          CHECK(parent->getChildren().empty());
          CHECK(child->getParent() == nullptr);
        }
      }
    }
  }

  GIVEN("Scene with Items in same place") {
    std::shared_ptr<svc::Scene> scene = std::make_shared<svc::Scene>();

    svc::Point initialPoint = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);

    svc::ItemPtr firstItem = std::make_shared<BasicItem>();
    firstItem->setScenePos(initialPoint);

    svc::ItemPtr secondItem = std::make_shared<BasicItem>();
    secondItem->setScenePos(initialPoint);

    scene->appendItem(firstItem);
    scene->appendItem(secondItem);

    WHEN("query by point") {
      svc::ItemList list = scene->query(initialPoint);

      THEN("list must contains both Items") {
        REQUIRE(list.size() == 2);
        CHECK(list.front() != list.back());
      }
    }
  }

  GIVEN("Scene with several Items in different places") {
    std::shared_ptr<svc::Scene> scene = std::make_shared<svc::Scene>();

    svc::ItemPtr firstItem = std::make_shared<BasicItem>();
    svc::Point   firstInitialPoint{10, 10};
    firstItem->setScenePos(firstInitialPoint);

    svc::ItemPtr secondItem = std::make_shared<BasicItem>();
    svc::Point   secondInitialPoint{50, 50};
    secondItem->setScenePos(secondInitialPoint);

    scene->appendItem(firstItem);
    scene->appendItem(secondItem);

    WHEN("get Item by point query") {
      svc::ItemList list = scene->query(firstInitialPoint);

      THEN("the list must contains one Item (first)") {
        REQUIRE(list.size() == 1);
        CHECK(list.front() == firstItem);
      }
    }

    WHEN("get Item by box query (intersects)") {
      WHEN("Box less then bounding box of Item") {
        svc::Box      box{{9, 9}, {11, 11}};
        svc::ItemList list =
            scene->query(box, svc::Scene::SpatialIndex::Intersects);

        THEN("list must contains first Item") {
          REQUIRE(list.size() == 1);
          CHECK(list.front() == firstItem);
        }
      }

      WHEN("Box intersects with bounding boxes of two Items") {
        svc::Box      box{firstItem->getScenePos(), secondItem->getScenePos()};
        svc::ItemList list =
            scene->query(box, svc::Scene::SpatialIndex::Intersects);

        THEN("list must contains both Items") {
          REQUIRE(list.size() == 2);
          CHECK(list.front() != list.back());
        }
      }

      WHEN("Box contains both Items") {
        svc::Box      box{{0, 0}, {60, 60}};
        svc::ItemList list =
            scene->query(box, svc::Scene::SpatialIndex::Intersects);

        THEN("list must contains both Items") {
          REQUIRE(list.size() == 2);
          CHECK(list.front() != list.back());
        }
      }
    }

    WHEN("get Item by box query (within)") {
      WHEN("Box less then bounding box of Item") {
        svc::Box      box{{9, 9}, {11, 11}};
        svc::ItemList list =
            scene->query(box, svc::Scene::SpatialIndex::Within);

        THEN("list must be empty") {
          REQUIRE(list.size() == 0);
        }
      }

      WHEN("Box intersects with bounding boxes of two Items") {
        svc::Box      box{firstItem->getScenePos(), secondItem->getScenePos()};
        svc::ItemList list =
            scene->query(box, svc::Scene::SpatialIndex::Within);

        THEN("list must be empty") {
          REQUIRE(list.size() == 0);
        }
      }

      WHEN("Box contains both Items") {
        svc::Box      box{{0, 0}, {60, 60}};
        svc::ItemList list =
            scene->query(box, svc::Scene::SpatialIndex::Within);

        THEN("list must contains both Items") {
          REQUIRE(list.size() == 2);
          CHECK(list.front() != list.back());
        }
      }
    }
  }

  GIVEN("Scene with one Item") {
    std::shared_ptr<svc::Scene> scene = std::make_shared<svc::Scene>();

    svc::ItemPtr item         = std::make_shared<BasicItem>();
    svc::Point   initialPoint = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    item->setScenePos(initialPoint);

    scene->appendItem(item);

    WHEN("set position") {
      svc::Point newPos = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);

      bool isScenePos = GENERATE(false, true);
      if (isScenePos) {
        item->setScenePos(newPos);
      } else {
        item->setPos(newPos);
      }

      THEN("Scene must contains only one Item") {
        CHECK(scene->count() == 1);
      }

      THEN("we must get the Item by new position") {
        svc::ItemList list = scene->query(newPos);

        CHECK(list.size() == 1);
      }

      THEN("if diff was more then diagonal of bounding box, then we can not"
           "get the Item by previous position") {
        float diag       = bq::mag(svc::Point{10, 10});
        float diffLenght = bq::mag(newPos - initialPoint);

        if (diag > diffLenght) {
          svc::ItemList list = scene->query(initialPoint);
          CHECK(list.size() == 0);
        }
      }
    }

    WHEN("rotade Item by some corner") {
      svc::Point startPos{20, 0};
      item->setScenePos(startPos);
      item->setSceneRotation(TO_RAD(90), {-20, 0});

      THEN("Scene has only one element") {
        CHECK(scene->count() == 1);
      }

      THEN("we can't get the Item by start position") {
        CHECK(scene->query(startPos).empty());
      }

      THEN("we can get the Item by new position") {
        svc::Point    newPos{0, 20};
        svc::ItemList list = scene->query(newPos);

        REQUIRE(list.size() == 1);
        CHECK(list.front() == item);
      }
    }
  }

  GIVEN("Scene with nested Items") {
    std::shared_ptr<svc::Scene> scene = std::make_shared<svc::Scene>();

    svc::ItemPtr first  = std::make_shared<BasicItem>();
    svc::ItemPtr second = std::make_shared<BasicItem>();
    svc::ItemPtr third  = std::make_shared<BasicItem>();
    svc::ItemPtr fourd  = std::make_shared<BasicItem>();

    first->appendChild(second);
    second->appendChild(third);
    third->appendChild(fourd);

    scene->appendItem(first);

    WHEN("set new position for root Item") {
      svc::Point newPos = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);
      first->setScenePos(newPos);

      THEN("now we can get all the Items by the position from query") {
        svc::ItemList list = scene->query(newPos);

        REQUIRE(list.size() == 4);
      }
    }
  }
}

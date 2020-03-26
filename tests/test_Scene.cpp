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
        svc::ItemPtr invalidItem;
        CHECK_THROWS(scene.removeItem(invalidItem));

        svc::ItemPtr itemWithoutScene = std::make_shared<BasicItem>();
        CHECK_THROWS(scene.removeItem(itemWithoutScene));
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
      scene->removeItem(item);

      THEN("now the Item don't associated with any Scene") {
        CHECK(item->getScene() == nullptr);
      }

      WHEN("try remove again") {
        THEN("produce error") {
          CHECK_THROWS(scene->removeItem(item));
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
        scene->removeItem(child);

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
        parent->removeChild(child);

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
        scene->removeItem(parent);

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
}

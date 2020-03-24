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

    WHEN("we try remove not existing item") {
      THEN("it must produce execption") {
        CHECK_THROWS(scene.removeItem({}));
        CHECK_THROWS(scene.removeItem(std::make_shared<BasicItem>()));
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
}

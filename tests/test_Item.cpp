// test_Item.cpp

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

//

#include <svc/AbstractItem.hpp>

#define CHECK_POINTS_EQUAL(first, second)                                      \
  CHECK(Approx{(first).x()} == (second).x());                                  \
  CHECK(Approx{(first).y()} == (second).y());

#define ANGLE_GENERATOR() TO_RAD(GENERATE(take(1, random(-360, 360))))

#define POINT_GENERATOR()                                                      \
  svc::Point {                                                                 \
    GENERATE(take(1, random(-1000.f, 1000.f))),                                \
        GENERATE(take(1, random(-1000.f, 1000.f)))                             \
  }

using ItemPtr = svc::ItemPtr;

class BasicItem final : public svc::AbstractItem {
public:
  svc::Box getBoundingBox() const noexcept override {
    return svc::Box{svc::Point{-5, -5}, svc::Point{5, 5}};
  }
};

SCENARIO("test Item", "[Item]") {
  GIVEN("basic Item") {
    ItemPtr    basicItem          = std::make_shared<BasicItem>();
    float      defaultParentAngle = ANGLE_GENERATOR();
    svc::Point defaultParentPos   = POINT_GENERATOR();
    basicItem->setRotation(defaultParentAngle);
    basicItem->setPos(defaultParentPos);

    THEN("item don't has associated Scene") {
      CHECK(basicItem->getScene() == nullptr);
    }

    THEN("item not has a parent") {
      CHECK(basicItem->getParent() == nullptr);
    }

    THEN("item don't has children") {
      CHECK(basicItem->getChildren().empty());
    }

    WHEN("change position") {
      svc::Point newPos = POINT_GENERATOR();
      basicItem->setPos(newPos);

      WHEN("position right") {
        svc::Point pos = basicItem->getPos();

        CHECK_POINTS_EQUAL(pos, newPos);

        THEN("scene position equal to position") {
          svc::Point scenePos = basicItem->getScenePos();

          CHECK_POINTS_EQUAL(pos, scenePos);
        }
      }
    }

    THEN("test rotation") {
      WHEN("rotate around default anchor") {
        float angle = ANGLE_GENERATOR();
        basicItem->rotate(angle);

        // THEN("angle of Item must be same") {
        //  float currentAngle = basicItem->getRotation();

        //  CHECK(Approx{currentAngle - prevAngle} == angle);
        //}

        THEN("position must be same, because we rotated the Item around its "
             "position") {
          svc::Point currentPos      = basicItem->getPos();
          svc::Point currentScenePos = basicItem->getScenePos();

          CHECK_POINTS_EQUAL(currentPos, defaultParentPos);
          CHECK_POINTS_EQUAL(currentScenePos, defaultParentPos);
        }

        basicItem->setRotation(angle);

        // THEN("angle of Item must be equal to seted angle") {
        //  float currentAngle = basicItem->getRotation();

        //  CHECK(Approx{currentAngle} == angle);
        //}

        THEN("position must be same, because we rotated the Item around its "
             "position") {
          svc::Point currentPos      = basicItem->getPos();
          svc::Point currentScenePos = basicItem->getScenePos();

          CHECK_POINTS_EQUAL(currentPos, defaultParentPos);
          CHECK_POINTS_EQUAL(currentScenePos, defaultParentPos);
        }
      }

      WHEN("rotation with anchor") {
        float      angle  = ANGLE_GENERATOR();
        svc::Point anchor = POINT_GENERATOR();
        basicItem->rotate(angle, anchor);

        THEN("check it") {
          svc::Point currentPos = basicItem->getPos();

          svc::Point diff = currentPos - defaultParentPos;
          // TODO
        }

        basicItem->setRotation(angle, anchor);

        THEN("check it") {
          // TODO
        }
      }
    }

    GIVEN("some child for Item") {
      ItemPtr    childItem         = std::make_shared<BasicItem>();
      float      defaultChildAngle = ANGLE_GENERATOR();
      svc::Point defaultChildPoint = POINT_GENERATOR();
      childItem->setRotation(defaultChildAngle);
      childItem->setPos(defaultChildPoint);

      basicItem->appendChild(childItem);

      svc::Point prevParentPos      = basicItem->getPos();
      svc::Point prevParentScenePos = basicItem->getScenePos();

      svc::Point prevChildPos      = childItem->getPos();
      svc::Point prevChildScenePos = childItem->getScenePos();

      THEN("child was be added to children") {
        CHECK(basicItem->getChildren().size() == 1);
      }
      THEN("child has parent equal to the Item") {
        CHECK(childItem->getParent() == basicItem.get());
      }
      THEN("scene position of the child must be same as before") {
        svc::Point currentChildScenePos = childItem->getScenePos();

        CHECK_POINTS_EQUAL(defaultChildPoint, currentChildScenePos);
      }

      WHEN("move parent") {
        svc::Point diff = POINT_GENERATOR();
        basicItem->moveOn(diff);

        THEN("position of child Item must not change, but changed Scene "
             "position") {
          svc::Point newChildPos      = childItem->getPos();
          svc::Point newSceneChildPos = childItem->getScenePos();

          CHECK_POINTS_EQUAL(prevChildPos, newChildPos);

          svc::Point childDiff = newSceneChildPos - prevChildScenePos;

          CHECK_POINTS_EQUAL(diff, childDiff);
        }
      }

      WHEN("set position for parent") {
        svc::Point newParentPos = POINT_GENERATOR();
        basicItem->setPos(newParentPos);

        svc::Point parentPosDiff = newParentPos - prevParentPos;

        THEN("child change its postion, but not Scene position") {
          svc::Point newChildPos = childItem->getPos();

          CHECK_POINTS_EQUAL(prevChildPos, newChildPos);

          svc::Point newChildScenePos = childItem->getScenePos();
          svc::Point childPosDiff     = newChildScenePos - prevChildScenePos;

          CHECK_POINTS_EQUAL(childPosDiff, parentPosDiff);
        }
      }

      GIVEN("new parent for child") {
        svc::ItemPtr newItem             = std::make_shared<BasicItem>();
        svc::Point   newItemScenePos     = POINT_GENERATOR();
        float        defaultNewItemAngle = ANGLE_GENERATOR();
        newItem->setScenePos(newItemScenePos);
        newItem->setRotation(defaultNewItemAngle);

        THEN("difference between scene position of parent and child must be "
             "equal to child position") {
          svc::Point diff = prevChildScenePos - prevParentScenePos;

          svc::Point childPos = childItem->getPos();

          CHECK_POINTS_EQUAL(childPos, diff);
        }

        WHEN("child change parent") {
          newItem->appendChild(childItem);

          THEN("child must has same Scene position as before, but new position "
               "relatively to parent") {
            svc::Point newChildPos      = childItem->getPos();
            svc::Point newChildScenePos = childItem->getScenePos();

            svc::Point diff = prevChildScenePos - newItemScenePos;

            CHECK_POINTS_EQUAL(newChildScenePos, prevChildScenePos);

            CHECK_POINTS_EQUAL(diff, newChildPos);
          }
        }
      }

      WHEN("remove child") {
        basicItem->removeChild(childItem);

        THEN("child removed from children") {
          CHECK(basicItem->getChildren().empty());
        }

        THEN("check that child don't has a parent") {
          CHECK(childItem->getParent() == nullptr);
        }
      }

      WHEN("remove parent") {
        basicItem.reset();

        THEN("child item must not has any parent") {
          CHECK(childItem->getParent() == nullptr);
        }
      }
    }
  }
}

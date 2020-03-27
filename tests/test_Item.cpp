// test_Item.cpp

// FIXME add tests for rotation, especially for Scene rotation

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

//

#include "test_auxilary.hpp"
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>
#include <svc/AbstractItem.hpp>

using ItemPtr = svc::ItemPtr;

class BasicItem final : public svc::AbstractItem {
public:
  svc::Box getBoundingBox() const noexcept override {
    return svc::Box{svc::Point{-5, -5}, svc::Point{5, 5}};
  }

  void accept([[maybe_unused]] svc::AbstractVisitor *visitor) override {
  }
};

SCENARIO("test Item", "[Item]") {
  GIVEN("one Item") {
    ItemPtr    basicItem    = std::make_shared<BasicItem>();
    float      defaultAngle = ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Point defaultPos   = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    basicItem->setRotation(defaultAngle);
    basicItem->setPos(defaultPos);

    THEN("item don't has associated Scene") {
      CHECK(basicItem->getScene() == nullptr);
    }
    THEN("item not has a parent") {
      CHECK(basicItem->getParent() == nullptr);
    }
    THEN("item don't has children") {
      CHECK(basicItem->getChildren().empty());
    }

    THEN("rotation angle and Scene rotation Angle are same") {
      float angle      = basicItem->getRotation();
      float sceneAngle = basicItem->getSceneRotation();

      CHECK_ANGLES_EQUAL(angle, sceneAngle);
      CHECK_ANGLES_EQUAL(defaultAngle, sceneAngle);
    }

    WHEN("set new position") {
      svc::Point newPos     = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);
      bool       isScenePos = GENERATE(false, true);
      if (isScenePos) {
        basicItem->setScenePos(newPos);
      } else {
        basicItem->setPos(newPos);
      }

      THEN("both Scene and relative position must be same") {
        svc::Point pos      = basicItem->getPos();
        svc::Point scenePos = basicItem->getScenePos();

        CHECK_POINTS_EQUAL(pos, scenePos);
        CHECK_POINTS_EQUAL(pos, newPos);
      }
    }

    WHEN("move position") {
      svc::Point vec = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);
      basicItem->moveOn(vec);

      THEN("Scene position and position must be same, because Item don't has a "
           "parent") {
        svc::Point scenePos = basicItem->getScenePos();
        svc::Point pos      = basicItem->getPos();

        CHECK_POINTS_EQUAL(scenePos, pos);
      }

      THEN("length of real diff postion must be same as lenght of vector") {
        svc::Point scenePos = basicItem->getScenePos();

        svc::Point diff = scenePos - defaultPos;

        CHECK(Approx{bq::mag(diff)} == bq::mag(vec));
      }

      THEN("check new position") {
        svc::Matrix rotationMat = bq::rotz_mat<3>(defaultAngle);
        svc::Point  sceneVec;
        boost::geometry::transform(
            vec,
            sceneVec,
            boost::geometry::strategy::transform::
                matrix_transformer<float, 2, 2>(rotationMat));

        svc::Point scenePos = basicItem->getScenePos();
        svc::Point diff     = scenePos - defaultPos;

        CHECK_POINTS_EQUAL(diff, sceneVec);
      }
    }

    WHEN("rotate around default anchor") {
      float angle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);
      basicItem->rotate(angle);

      THEN("angle of Item must be sum of default and rotation angles") {
        float currentAngle = basicItem->getRotation();

        CHECK_ANGLES_EQUAL(currentAngle - defaultAngle, angle);
      }

      THEN("position must be same, because we rotated the Item around its "
           "position") {
        svc::Point currentPos      = basicItem->getPos();
        svc::Point currentScenePos = basicItem->getScenePos();

        CHECK_POINTS_EQUAL(currentPos, defaultPos);
        CHECK_POINTS_EQUAL(currentScenePos, defaultPos);
      }
    }

    WHEN("set rotation with default anchor") {
      float angle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);
      basicItem->setRotation(angle);

      THEN("angle of Item must be equal to the angle") {
        float currentAngle = basicItem->getRotation();

        CHECK_ANGLES_EQUAL(currentAngle, angle);
      }

      THEN("position must be same, because we rotated the Item around its "
           "position") {
        svc::Point currentPos      = basicItem->getPos();
        svc::Point currentScenePos = basicItem->getScenePos();

        CHECK_POINTS_EQUAL(currentPos, defaultPos);
        CHECK_POINTS_EQUAL(currentScenePos, defaultPos);
      }
    }

    WHEN("set Scene rotation") {
      float angle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);
      basicItem->setSceneRotation(angle);

      THEN("Scene angle and simple angle must be equal to setted angle") {
        float currentAngle      = basicItem->getRotation();
        float currentSceneAngle = basicItem->getSceneRotation();

        CHECK_ANGLES_EQUAL(currentAngle, currentSceneAngle);
        CHECK_ANGLES_EQUAL(currentAngle, angle);
      }
    }

    WHEN("rotate with anchor") {
      float      angle  = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);
      svc::Point anchor = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);
      basicItem->rotate(angle, anchor);

      THEN("rotation of the Item must be a sum of ratation of the angle and "
           "default Item angle") {
        float currentAngle = basicItem->getRotation(anchor);

        CHECK_ANGLES_EQUAL(currentAngle - defaultAngle, angle);
      }
    }

    WHEN("set rotation with anchor") {
      float      angle  = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);
      svc::Point anchor = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);
      basicItem->setRotation(angle, anchor);

      THEN("rotation of the Item must be same to angle") {
        float currentAngle = basicItem->getRotation();

        CHECK_ANGLES_EQUAL(currentAngle, angle);
      }
    }
  }

  GIVEN("parent and child Item-s") {
    ItemPtr parentItem = std::make_shared<BasicItem>();
    ItemPtr childItem  = std::make_shared<BasicItem>();

    float      defaultParentAngle     = ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Point defaultParentScenePos  = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    float      defaultChildSceneAngle = ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Point defaultChildScenePos   = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);

    parentItem->setRotation(defaultParentAngle);
    parentItem->setScenePos(defaultParentScenePos);

    childItem->setRotation(defaultChildSceneAngle);
    childItem->setScenePos(defaultChildScenePos);

    WHEN("add child to parent") {
      parentItem->appendChild(childItem);

      svc::Point defaultParentPos = parentItem->getPos();
      svc::Point defaultChildPos  = childItem->getPos();

      float defaultChildAngle = childItem->getRotation();

      THEN("child was be added to children") {
        CHECK(parentItem->getChildren().size() == 1);
      }
      THEN("child has parent") {
        CHECK(childItem->getParent() == parentItem.get());
      }
      THEN("scene position and scene angle of the child must be same as "
           "before") {
        svc::Point currentChildScenePos   = childItem->getScenePos();
        float      currentChildSceneAngle = childItem->getSceneRotation();

        CHECK_POINTS_EQUAL(defaultChildScenePos, currentChildScenePos);

        CHECK_ANGLES_EQUAL(currentChildSceneAngle, defaultChildSceneAngle);
      }
      THEN("vector between scene position of parent and child must have equal "
           "length") {
        svc::Point diff = defaultChildScenePos - defaultParentScenePos;

        svc::Point childPos = childItem->getPos();

        CHECK(Approx{bq::mag(childPos)} == bq::mag(diff));
      }

      WHEN("move parent") {
        svc::Point diff = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);
        parentItem->moveOn(diff);

        THEN("position of child Item must not change, but changed Scene "
             "position") {
          svc::Point newChildPos      = childItem->getPos();
          svc::Point newSceneChildPos = childItem->getScenePos();

          CHECK_POINTS_EQUAL(defaultChildPos, newChildPos);

          svc::Point childDiff = newSceneChildPos - defaultChildScenePos;

          CHECK(Approx{bq::mag(diff)} == bq::mag(childDiff));
        }
      }

      WHEN("set position for parent") {
        svc::Point newParentPos = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);
        parentItem->setPos(newParentPos);

        svc::Point parentPosDiff = newParentPos - defaultParentPos;

        THEN("child change its postion, but not Scene position") {
          svc::Point childPos = childItem->getPos();

          CHECK_POINTS_EQUAL(defaultChildPos, childPos);

          svc::Point newChildScenePos = childItem->getScenePos();
          svc::Point childPosDiff     = newChildScenePos - defaultChildScenePos;

          CHECK_POINTS_EQUAL(childPosDiff, parentPosDiff);
        }
      }

      WHEN("rorate parent") {
        float angle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);
        parentItem->rotate(angle);

        THEN("child not change its angle and position relative to parent, but "
             "can change angle (if it's not 0) and position relative to "
             "Scene") {
          svc::Point currentChildPos   = childItem->getPos();
          float      currentChildAngle = childItem->getRotation();

          CHECK_POINTS_EQUAL(currentChildPos, defaultChildPos);
          CHECK_ANGLES_EQUAL(currentChildAngle, defaultChildAngle);
        }

        THEN("child Scene rotation must be change as parent rotation") {
          // XXX if angle is 0 it is a problem for checking, becuase we can not
          // approximate to 0
          if (angle != 0) {
            float currentSceneAngle = childItem->getSceneRotation();

            float childDiff = currentSceneAngle - defaultChildSceneAngle;

            CHECK_ANGLES_EQUAL(childDiff, angle);
          }
        }
      }

      WHEN("remove child") {
        parentItem->removeChild(childItem);

        THEN("child removed from children") {
          CHECK(parentItem->getChildren().empty());
        }

        THEN("check that child don't has a parent") {
          CHECK(childItem->getParent() == nullptr);
        }

        WHEN("try remove again") {
          THEN("produce exeption") {
            CHECK_THROWS(parentItem->removeChild(childItem));
          }
        }
      }

      WHEN("set Scene rotation angle for child") {
        float angle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);
        childItem->setSceneRotation(angle);

        THEN("Scene rotation angle of the child must be equal to setted") {
          float currentAngle = childItem->getSceneRotation();

          // XXX because in case of angle == 0 we can not approximate
          if (angle != 0) {
            CHECK_ANGLES_EQUAL(currentAngle, angle);
          }
        }
      }

      WHEN("remove parent") {
        parentItem.reset();

        THEN("child item must not has any parent") {
          CHECK(childItem->getParent() == nullptr);
        }
        THEN("child must has equal position and Scene position") {
          svc::Point currentPos      = childItem->getPos();
          svc::Point currentScenePos = childItem->getScenePos();
          float      angle           = childItem->getSceneRotation();

          CHECK_POINTS_EQUAL(currentPos, currentScenePos);

          CHECK_POINTS_EQUAL(currentScenePos, defaultChildScenePos);

          CHECK_ANGLES_EQUAL(angle, defaultChildSceneAngle);
        }
      }

      WHEN("try remove wrong child") {
        THEN("it must produce exception") {
          CHECK_THROWS(childItem->removeChild(parentItem));
          CHECK_THROWS(parentItem->removeChild(parentItem));

          svc::ItemPtr invalidItem;
          CHECK_THROWS(parentItem->removeChild(invalidItem));
        }
      }
    }
  }

  GIVEN("parent and child Item-s") {
    svc::ItemPtr parentItem = std::make_shared<BasicItem>();
    svc::ItemPtr childItem  = std::make_shared<BasicItem>();

    svc::Point parentScenePos = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Point childScenePos  = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);

    float parentAngle = ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    float childAngle  = ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);

    parentItem->setScenePos(parentScenePos);
    parentItem->setRotation(parentAngle);

    childItem->setScenePos(childScenePos);
    childItem->setRotation(childAngle);

    parentItem->appendChild(childItem);

    svc::Point childPos = childItem->getPos();

    GIVEN("third Item") {
      svc::ItemPtr thirdItem = std::make_shared<BasicItem>();

      svc::Point defaultThirdScenePos = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);
      float      defaultThirdAngle    = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);

      thirdItem->setScenePos(defaultThirdScenePos);
      thirdItem->setRotation(defaultThirdAngle);

      WHEN("add new child for parent") {
        parentItem->appendChild(thirdItem);

        THEN("parent has two children") {
          CHECK(parentItem->getChildren().size() == 2);
        }
      }

      WHEN("child change parent") {
        thirdItem->appendChild(childItem);

        THEN("parent Item already not has any children") {
          CHECK(parentItem->getChildren().empty());
        }
        THEN("third Item has one child") {
          CHECK(thirdItem->getChildren().size() == 1);
        }
        THEN("child has new parent") {
          CHECK(childItem->getParent() == thirdItem.get());
        }

        THEN("child must has same Scene position as before, but new position "
             "relatively to parent") {
          svc::Point newChildPos      = childItem->getPos();
          svc::Point newChildScenePos = childItem->getScenePos();

          CHECK_POINTS_EQUAL(newChildScenePos, childScenePos);

          svc::Point diff = childScenePos - defaultThirdScenePos;

          CHECK(Approx{bq::mag(diff)}.epsilon(0.01) == bq::mag(newChildPos));
        }
      }

      WHEN("add parent for parent Item") {
        thirdItem->appendChild(parentItem);

        THEN("parent now also has a parent") {
          CHECK(parentItem->getParent() == thirdItem.get());
        }

        THEN("child don't change its position (relative to parent or relative "
             "to Scene)") {
          svc::Point currentPos      = childItem->getPos();
          svc::Point currentScenePos = childItem->getScenePos();

          CHECK_POINTS_EQUAL(currentPos, childPos);
          CHECK_POINTS_EQUAL(currentScenePos, childScenePos);
        }
      }
    }
  }
}

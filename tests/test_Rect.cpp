// test_Rect.cpp

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

//

#include "svc/base_geometry_types.hpp"
#include "test_auxilary.hpp"
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/qvm/swizzle.hpp>

SCENARIO("test Rect", "[Rect]") {
  GIVEN("simple Rect without rotation") {
    svc::Point minCorner = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Size  size{10, 10};

    svc::Rect rect{minCorner, size, 0};

    THEN("check minCorner and size") {
      CHECK_POINTS_EQUAL(minCorner, rect.getMinCorner());
      CHECK_SIZES_EQUAL(size, rect.size());
    }

    THEN("rotatin must be equal to 0") {
      CHECK_ANGLES_EQUAL(rect.getRotation(), 0);
    }

    WHEN("set rotation around default corner") {
      float angle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);

      rect.setRotation(angle);

      THEN("rotation must be same as we set") {
        CHECK_ANGLES_EQUAL(rect.getRotation(), angle);
      }

      THEN("minCorner must be as previous") {
        CHECK_POINTS_EQUAL(rect.getMinCorner(), minCorner);
      }
    }

    WHEN("set roation around some corner") {
      float      angle  = TO_RAD(90);
      svc::Point anchor = {5, 0};

      rect.setRotation(angle, anchor);

      THEN("angle must be same as we set") {
        CHECK_ANGLES_EQUAL(rect.getRotation(), angle);
      }

      THEN("minCorner changed") {
        svc::Point currentMinCorner = rect.getMinCorner();
        svc::Point mustBeCorner     = minCorner + svc::Point{0, -5};

        CHECK_POINTS_EQUAL(mustBeCorner, currentMinCorner);
      }
    }

    WHEN("set new minCorner") {
      svc::Point newMinCorner = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);

      rect.setMinCorner(newMinCorner);

      THEN("check new minCorner") {
        CHECK_POINTS_EQUAL(rect.getMinCorner(), newMinCorner);
      }
    }

    WHEN("set new position after rotating") {
      float angle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);

      rect.setRotation(TO_RAD(angle));

      svc::Point newMinCorner = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);

      rect.setMinCorner(newMinCorner);

      THEN("minCorner must be equal to point that was set") {
        CHECK_POINTS_EQUAL(rect.getMinCorner(), newMinCorner);
      }
    }

    WHEN("move on some vector") {
      svc::Point vec = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);

      rect.moveOn(vec);

      THEN("new minCorner must be as previous + vec") {
        svc::Point currentMinCorner = rect.getMinCorner();

        CHECK_POINTS_EQUAL(currentMinCorner, minCorner + vec);
      }
    }

    WHEN("move on some vector after rotation") {
      float angle = TO_RAD(90);
      rect.setRotation(angle);

      svc::Point vec = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);

      rect.moveOn(vec);

      THEN("then check new minCorner") {
        svc::Point realVec{-vec.y(), vec.x()}; // rotate on 90 deg

        svc::Point currentMinCorner = rect.getMinCorner();

        CHECK_POINTS_EQUAL(currentMinCorner, minCorner + realVec);
      }
    }
  }

  GIVEN("rect with some rotation") {
    float      angle     = ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Point minCorner = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Size  size      = {10, 10};

    svc::Rect rect{minCorner, size, angle};

    THEN("check that rotation is right") {
      CHECK_ANGLES_EQUAL(rect.getRotation(), angle);
    }

    WHEN("rotate Rect") {
      float rotateOn = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);

      rect.rotate(rotateOn);

      THEN("check that new angle will be as sum of rotation and previous "
           "angle") {
        float mustBeAngle = angle + rotateOn;

        CHECK_ANGLES_EQUAL(rect.getRotation(), mustBeAngle);
      }
    }

    WHEN("transform the Rect to Ring") {
      svc::Ring ring = rect;

      THEN("ring must be valid") {
        CHECK(boost::geometry::is_valid(ring));
      }

      THEN("ring must has 4 points") {
        CHECK(ring.size() == 4);
      }

      THEN("output ring must has same area as rect") {
        float ringArea = bg::area(ring);

        float rectArea = bg::area(svc::Box{{0, 0}, svc::Point(rect.size())});

        CHECK(Approx(ringArea).epsilon(0.01) == rectArea);
      }

      THEN("first point must be equal to minCorner and third must be equal to "
           "maxCorner of rect") {
        CHECK_POINTS_EQUAL(ring[0], rect.getMinCorner());

        svc::Point  rectSize = rect.size();
        svc::Matrix rectMat  = rect.getMatrix();
        svc::Point  maxCorner =
            boost::qvm::XY(rectMat * boost::qvm::XY1(rectSize));

        CHECK_POINTS_EQUAL(ring[2], maxCorner);
      }
    }
  }
}

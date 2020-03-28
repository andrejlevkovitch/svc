// test_View.cpp

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

//

#include "svc/AbstractView.hpp"
#include "test_auxilary.hpp"

class View final : public svc::AbstractView {
public:
  svc::Size size() const noexcept override {
    return size_;
  }

  void setSize(svc::Size newSize) noexcept {
    size_ = newSize;
  }

  using AbstractView::mapToScene;

private:
  svc::Size size_ = {100, 100};
};

SCENARIO("test View", "[View]") {
  GIVEN("empty view") {
    View view;

    THEN("check that scene not set") {
      CHECK(view.getScene() == nullptr);
    }

    THEN("check that default rect is same to view rect") {
      svc::Rect sceneRect = view.getSceneRect();

      svc::Point zeroPoint{0, 0};
      CHECK_POINTS_EQUAL(sceneRect.getMinCorner(), zeroPoint);

      CHECK_SIZES_EQUAL(sceneRect.size(), view.size());

      CHECK_ANGLES_EQUAL(sceneRect.getRotation(), 0);
    }

    WHEN("increase size of view") {
      view.setSize(view.size() + svc::Size{10, 10});

      THEN("size of scene rect also changed") {
        svc::Rect rect = view.getSceneRect();

        CHECK_SIZES_EQUAL(rect.size(), view.size());
      }
    }
  }

  GIVEN("View and some scene rect") {
    View view;

    svc::Point minCorner = POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Size  rectSize  = SIZE_GENERATOR(FIRST_LEVEL_GENERATOR);
    float      rectAngle = ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Rect  rect      = {minCorner, rectSize, rectAngle};

    view.setSceneRect(rect);

    THEN("check that scene rect are same as set") {
      svc::Rect sceneRect = view.getSceneRect();

      CHECK_POINTS_EQUAL(sceneRect.getMinCorner(), minCorner);

      CHECK_SIZES_EQUAL(sceneRect.size(), rectSize);

      CHECK_ANGLES_EQUAL(sceneRect.getRotation(), rectAngle);
    }

    WHEN("change size of view on N%") {
      svc::Size viewSize = view.size();
      svc::Size newSize  = {viewSize.width() * 1.1f, viewSize.height() * 1.1f};

      view.setSize(newSize);

      THEN("minCorner and rotation didn't change") {
        CHECK_POINTS_EQUAL(rect.getMinCorner(),
                           view.getSceneRect().getMinCorner());

        CHECK_ANGLES_EQUAL(view.getSceneRect().getRotation(), rectAngle);
      }

      THEN("size of scene rect also change on N%") {
        svc::Size mustBeSize = {rect.size().width() * 1.1f,
                                rect.size().height() * 1.1f};

        svc::Rect currentRect = view.getSceneRect();

        CHECK_SIZES_EQUAL(mustBeSize, currentRect.size());
      }
    }
  }

  GIVEN("View") {
    View view;

    svc::Point minCorner = {0, 0}; // POINT_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Size  rectSize = view.size(); // SIZE_GENERATOR(FIRST_LEVEL_GENERATOR);
    float      angle    = 0; // ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Rect  rect     = {minCorner, rectSize, angle};

    view.setSceneRect(rect);

    WHEN("move scene rect") {
      svc::Point diff = POINT_GENERATOR(SECOND_LEVEL_GENERATOR);

      svc::Point mustBeMinCorner = view.mapToScene(diff);

      view.moveSceneRect(diff);

      THEN("angle and size din't change") {
        CHECK_ANGLES_EQUAL(angle, view.getSceneRect().getRotation());
        CHECK_SIZES_EQUAL(rectSize, view.getSceneRect().size());
      }

      THEN("minCorner changed") {
        CHECK_POINTS_EQUAL(mustBeMinCorner, view.getSceneRect().getMinCorner());
      }
    }

    WHEN("rotate scene rect around default anchor") {
      float newAngle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);

      view.getSceneRect();

      view.rotateSceneRect(newAngle);

      THEN("minCorner and size didn't change") {
        CHECK_POINTS_EQUAL(minCorner, view.getSceneRect().getMinCorner());
        CHECK_SIZES_EQUAL(rectSize, view.getSceneRect().size());
      }

      THEN("new angle of rect must be as sum of previous and new") {
        CHECK_ANGLES_EQUAL(angle + newAngle, view.getSceneRect().getRotation());
      }
    }

    WHEN("rotate scene rect around some anchor") {
      float newAngle = ANGLE_GENERATOR(SECOND_LEVEL_GENERATOR);

      view.rotateSceneRect(newAngle, view.size() / 2); // by center of the view

      THEN("new angle of rect must be as sum of previous and new") {
        CHECK_ANGLES_EQUAL(angle + newAngle, view.getSceneRect().getRotation());
      }

      THEN("size didn't change") {
        CHECK_SIZES_EQUAL(rectSize, view.getSceneRect().size());
      }
    }

    WHEN("scale scene rect around default anchor") {
      view.scaleSceneRect({2, 2});

      THEN("position and angle didn't change") {
        CHECK_ANGLES_EQUAL(angle, view.getSceneRect().getRotation());
        CHECK_POINTS_EQUAL(minCorner, view.getSceneRect().getMinCorner());
      }

      THEN("size of scene rect incresed on 2") {
        CHECK_SIZES_EQUAL(rectSize * 2, view.getSceneRect().size());
      }
    }

    WHEN("scale scene rect around some anchor") {
      view.scaleSceneRect({2, 2}, view.size() / 2); // by center of the view

      THEN("angle didn't change") {
        CHECK_ANGLES_EQUAL(angle, view.getSceneRect().getRotation());
      }

      THEN("size increse on 2 and minCorner change position on 0.5 of size") {
        CHECK_SIZES_EQUAL(rectSize * 2, view.getSceneRect().size());
        CHECK_POINTS_EQUAL(minCorner - svc::Point(rectSize * 0.5),
                           view.getSceneRect().getMinCorner());
      }
    }
  }
}

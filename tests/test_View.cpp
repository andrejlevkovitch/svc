// test_View.cpp

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

//

#include "svc/AbstractItem.hpp"
#include "svc/AbstractView.hpp"
#include "svc/Scene.hpp"
#include "test_auxilary.hpp"

class View final : public svc::AbstractView {
public:
  svc::Size getContextSize() const noexcept override {
    return size_;
  }

  void setSize(svc::Size newSize) noexcept {
    size_ = newSize;
  }

  using AbstractView::mapToScene;

private:
  svc::Size size_ = {100, 100};
};

class BasicItem;

namespace svc {
class AbstractVisitor {
public:
  virtual ~AbstractVisitor()          = default;
  virtual void visit(BasicItem *item) = 0;
};
} // namespace svc

class BasicItem final : public svc::AbstractItem {
public:
  svc::Box getBoundingBox() const noexcept override {
    return {{-5, -5}, {5, 5}};
  }

  void accept([[maybe_unused]] svc::AbstractVisitor *visitor) override {
    visitor->visit(this);
  }

  using AbstractItem::setMatrix;
};

class CountVisitor final : public svc::AbstractVisitor {
public:
  CountVisitor()
      : count_{0} {
  }

  void visit([[maybe_unused]] BasicItem *item) override {
    ++count_;
  }

  int count_;
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

      CHECK_SIZES_EQUAL(sceneRect.size(), view.getContextSize());

      CHECK_ANGLES_EQUAL(sceneRect.getRotation(), 0);
    }

    WHEN("increase size of view") {
      view.setSize(view.getContextSize() + svc::Size{10, 10});

      THEN("size of scene rect also changed") {
        svc::Rect rect = view.getSceneRect();

        CHECK_SIZES_EQUAL(rect.size(), view.getContextSize());
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
      svc::Size viewSize = view.getContextSize();
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
    svc::Size  rectSize =
        view.getContextSize(); // SIZE_GENERATOR(FIRST_LEVEL_GENERATOR);
    float     angle = 0;       // ANGLE_GENERATOR(FIRST_LEVEL_GENERATOR);
    svc::Rect rect  = {minCorner, rectSize, angle};

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

      view.rotateSceneRect(newAngle,
                           view.getContextSize() / 2); // by center of the view

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
      view.scaleSceneRect({2, 2},
                          view.getContextSize() / 2); // by center of the view

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

  GIVEN("View and Scene with several Items") {
    View          view;
    svc::ScenePtr scene = std::make_shared<svc::Scene>();

    svc::ItemPtr item1 = std::make_shared<BasicItem>();
    svc::ItemPtr item2 = std::make_shared<BasicItem>();
    svc::ItemPtr item3 = std::make_shared<BasicItem>();
    svc::ItemPtr item4 = std::make_shared<BasicItem>();

    // item 1 and item4 - items of main level (withou parents)
    item1->appendChild(item2);
    item2->appendChild(item3);

    scene->appendItem(item1);
    scene->appendItem(item4);

    item1->setScenePos(svc::Point{10, 10});
    item2->setScenePos(svc::Point{50, 50});
    item3->setScenePos(svc::Point{-50, -50});
    item4->setScenePos(svc::Point{100, 100});

    view.setScene(scene);

    WHEN("set bounded rect as Scene Rect for view") {
      svc::Box bounded = scene->bounds();
      view.setSceneRect(svc::Rect(bounded));

      THEN("we can visit all Items on Scene") {
        CountVisitor counter;
        view.accept(&counter);

        CHECK(counter.count_ == 4);
      }
    }

    WHEN("set rect only for Items in first quarter") {
      svc::Rect rect{svc::Point{0, 0}, svc::Size{1000, 1000}, 0};
      view.setSceneRect(rect);

      THEN("we get only 3 items by counter") {
        CountVisitor counter;
        view.accept(&counter);

        CHECK(counter.count_ == 3);
      }
    }

    WHEN("set rect only for item1") {
      svc::Rect rect{svc::Point{0, 0}, svc::Size{20, 20}, 0};
      view.setSceneRect(rect);

      THEN("we get only one Item (item1 withou children)") {
        CountVisitor counter;
        view.accept(&counter);

        CHECK(counter.count_ == 1);
      }
    }

    WHEN("set rotation rect") {
      svc::Rect rect{svc::Point{0, 0}, svc::Size{1000, 1000}, TO_RAD(180)};
      view.setSceneRect(rect);

      THEN("we get only on Item (item3)") {
        CountVisitor counter;
        view.accept(&counter);

        CHECK(counter.count_ == 1);
      }
    }
  }
}

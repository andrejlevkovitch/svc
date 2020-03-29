// test_serialization.cpp

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

//

#include "svc/AbstractItem.hpp"
#include "svc/Scene.hpp"
#include "svc/serialization.hpp"
#include "test_auxilary.hpp"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <sstream>

class Item1;
class Item2;

namespace svc {
class AbstractVisitor {
public:
  virtual ~AbstractVisitor() = default;

  virtual void visit(Item1 *) = 0;
  virtual void visit(Item2 *) = 0;
};
} // namespace svc

namespace boost::serialization {
template <typename Archive>
void serialize(Archive &ar, boost::uuids::uuid &uuid, const unsigned int) {
  ar &uuid.data;
}
} // namespace boost::serialization

class Item1 final : public svc::AbstractItem {
public:
  Item1()
      : svc::AbstractItem{} {
    index = boost::uuids::random_generator_mt19937{}();
  }

  static std::string typeName() {
    return "Item1";
  }

  void accept(svc::AbstractVisitor *visitor) {
    visitor->visit(this);
  }

  svc::Box getBoundingBox() const noexcept {
    return svc::Box{{-5, -5}, {5, 5}};
  }

  template <typename Archive>
  void serialize(Archive &ar, const unsigned int) {
    ar &index;
    ar &boost::serialization::base_object<svc::AbstractItem>(*this);
  }

  boost::uuids::uuid index;
};

class Item2 final : public svc::AbstractItem {
public:
  Item2()
      : svc::AbstractItem{} {
    uuid = boost::uuids::random_generator_mt19937{}();
  }

  static std::string typeName() {
    return "Item2";
  }

  void accept(svc::AbstractVisitor *visitor) {
    visitor->visit(this);
  }

  svc::Box getBoundingBox() const noexcept {
    return svc::Box{{-5, -5}, {5, 5}};
  }

  template <typename Archive>
  void serialize(Archive &ar, const unsigned int) {
    ar &uuid;
    ar &boost::serialization::base_object<svc::AbstractItem>(*this);
  }

  boost::uuids::uuid uuid;
};

using StoredInfo = std::unordered_map<
    boost::uuids::uuid,
    std::tuple<std::string, svc::Point, float, svc::Point, float>>;

class StoreVisitor final : public svc::AbstractVisitor {
public:
  void visit(Item1 *item) override {
    boost::uuids::uuid uuid = item->index;

    std::string typeName   = item->typeName();
    svc::Point  pos        = item->getPos();
    float       angle      = item->getRotation();
    svc::Point  scenePos   = item->getScenePos();
    float       sceneAngle = item->getSceneRotation();

    info.emplace(uuid,
                 std::make_tuple(typeName, pos, angle, scenePos, sceneAngle));

    svc::ItemList children = item->getChildren();
    std::for_each(children.begin(),
                  children.end(),
                  [this](svc::ItemPtr &child) {
                    child->accept(this);
                  });
  }

  void visit(Item2 *item) override {
    boost::uuids::uuid uuid = item->uuid;

    std::string typeName   = item->typeName();
    svc::Point  pos        = item->getPos();
    float       angle      = item->getRotation();
    svc::Point  scenePos   = item->getScenePos();
    float       sceneAngle = item->getSceneRotation();

    info.emplace(uuid,
                 std::make_tuple(typeName, pos, angle, scenePos, sceneAngle));

    svc::ItemList children = item->getChildren();
    std::for_each(children.begin(),
                  children.end(),
                  [this](svc::ItemPtr &child) {
                    child->accept(this);
                  });
  }

  StoredInfo info;
};

BOOST_CLASS_EXPORT_GUID(Item1, "Item1")
BOOST_CLASS_EXPORT_GUID(Item2, "Item2")

#define CHECK_INFO(newInfo, storedInfoa)                                       \
  {                                                                            \
    REQUIRE(newInfo.size() == storedInfo.size());                              \
                                                                               \
    for (auto [uuid, info] : storedInfo) {                                     \
      auto nInfo = newInfo[uuid];                                              \
                                                                               \
      CHECK(std::get<0>(info) == std::get<0>(nInfo));                          \
      CHECK_POINTS_EQUAL(std::get<1>(info), std::get<1>(nInfo));               \
      CHECK_ANGLES_EQUAL(std::get<2>(info), std::get<2>(nInfo));               \
      CHECK_POINTS_EQUAL(std::get<3>(info), std::get<3>(nInfo));               \
      CHECK_ANGLES_EQUAL(std::get<4>(info), std::get<4>(nInfo));               \
    }                                                                          \
  }

SCENARIO("test Scene serialization", "[serialization]") {
  GIVEN("Scene with Items") {
    svc::Scene scene;

    size_t itemsCount = 100;
    bool   isNested   = GENERATE(false, true);
    if (isNested) {
      for (size_t i = 0; i < itemsCount / 4; ++i) {
        svc::ItemPtr item1 = std::make_shared<Item1>();
        svc::ItemPtr item2 = std::make_shared<Item2>();
        svc::ItemPtr item3 = std::make_shared<Item1>();
        svc::ItemPtr item4 = std::make_shared<Item2>();

        svc::Point pos1   = POINT_GENERATOR(THIRD_LEVEL_GENERATOR);
        float      angle1 = ANGLE_GENERATOR(THIRD_LEVEL_GENERATOR);
        svc::Point pos2   = POINT_GENERATOR(THIRD_LEVEL_GENERATOR);
        float      angle2 = ANGLE_GENERATOR(THIRD_LEVEL_GENERATOR);
        svc::Point pos3   = POINT_GENERATOR(THIRD_LEVEL_GENERATOR);
        float      angle3 = ANGLE_GENERATOR(THIRD_LEVEL_GENERATOR);
        svc::Point pos4   = POINT_GENERATOR(THIRD_LEVEL_GENERATOR);
        float      angle4 = ANGLE_GENERATOR(THIRD_LEVEL_GENERATOR);

        item1->setScenePos(pos1);
        item1->setSceneRotation(angle1);
        item2->setScenePos(pos2);
        item2->setSceneRotation(angle2);
        item3->setScenePos(pos3);
        item3->setSceneRotation(angle3);
        item4->setScenePos(pos4);
        item4->setSceneRotation(angle4);

        item1->appendChild(item2);
        item1->appendChild(item3);
        item3->appendChild(item4);

        scene.appendItem(item1);
      }
    } else {
      for (size_t i = 0; i < itemsCount / 2; ++i) {
        svc::ItemPtr item  = std::make_shared<Item1>();
        svc::Point   pos   = POINT_GENERATOR(THIRD_LEVEL_GENERATOR);
        float        angle = ANGLE_GENERATOR(THIRD_LEVEL_GENERATOR);
        item->setPos(pos);
        item->setRotation(angle);

        scene.appendItem(item);
      }

      for (size_t i = 0; i < itemsCount / 2; ++i) {
        svc::ItemPtr item  = std::make_shared<Item2>();
        svc::Point   pos   = POINT_GENERATOR(THIRD_LEVEL_GENERATOR);
        float        angle = ANGLE_GENERATOR(THIRD_LEVEL_GENERATOR);
        item->setPos(pos);
        item->setRotation(angle);

        scene.appendItem(item);
      }
    }

    REQUIRE(scene.count() == itemsCount);

    StoredInfo storedInfo;
    {
      StoreVisitor visitor;
      scene.accept(&visitor);
      storedInfo = visitor.info;
    }

    REQUIRE(storedInfo.size() == itemsCount);

    WHEN("serialize the Scene") {
      std::stringstream ss;

      {
        boost::archive::text_oarchive ar{ss};
        ar &                          scene;
      }

      THEN("initially Scene not changed") {
        StoredInfo newInfo;
        {
          StoreVisitor visitor;
          scene.accept(&visitor);
          newInfo = visitor.info;
        }

        CHECK_INFO(newInfo, storedInfo);
      }

      WHEN("restore archive to new Scene") {
        svc::Scene newScene;

        {
          boost::archive::text_iarchive ar{ss};
          ar &                          newScene;
        }

        THEN("info from new Scene must be equal to info from old scene") {
          StoredInfo newInfo;

          {
            StoreVisitor visitor;
            scene.accept(&visitor);
            newInfo = visitor.info;
          }

          CHECK_INFO(newInfo, storedInfo);
        }
      }
    }
  }
}

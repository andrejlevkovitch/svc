// serialization.hpp
/**\file using boost::serialization for serialize Scene and Items
 *
 * \note if you want to serialize your's Items, then you need define macro
 * BOOST_CLASS_EXPORT_GUID for your Items classes
 */

#pragma once

#include "AbstractItem.hpp"
#include "Scene.hpp"
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/version.hpp>

namespace boost::serialization {
template <typename Archive>
void serialize(Archive &                           ar,
               svc::Matrix &                       mat,
               [[maybe_unused]] const unsigned int version) {
  ar &mat.a;
}

template <typename Archive>
void save(Archive &                           ar,
          const svc::AbstractItem &           item,
          [[maybe_unused]] const unsigned int version) {
  ar &item.getMatrix();
  ar &item.getChildren();
}

template <typename Archive>
void load(Archive &                           ar,
          svc::AbstractItem &                 item,
          [[maybe_unused]] const unsigned int version) {
  svc::Matrix   mat;
  svc::ItemList children;
  ar &          mat;
  ar &          children;

  item.setMatrix(std::move(mat));
  for (svc::ItemPtr &child : children) {
    item.appendChild(child);
  }
}

template <typename Archive>
void save(Archive &                           ar,
          const svc::Scene &                  scene,
          [[maybe_unused]] const unsigned int version) {
  svc::ItemList rootItems = scene.rootItems();
  ar &          rootItems;
}

template <typename Archive>
void load(Archive &                           ar,
          svc::Scene &                        scene,
          [[maybe_unused]] const unsigned int version) {
  svc::ItemList rootItems;
  ar &          rootItems;

  for (svc::ItemPtr &item : rootItems) {
    scene.appendItem(item);
  }
}
} // namespace boost::serialization

BOOST_SERIALIZATION_ASSUME_ABSTRACT(svc::AbstractItem)
BOOST_SERIALIZATION_SPLIT_FREE(svc::AbstractItem)
BOOST_CLASS_VERSION(svc::AbstractItem, 1)

BOOST_SERIALIZATION_SPLIT_FREE(svc::Scene)
BOOST_CLASS_VERSION(svc::Scene, 1)

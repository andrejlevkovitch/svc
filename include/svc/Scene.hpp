// Scene.hpp
/**\file
 */

#pragma once

#include "svc/base_geometry_types.hpp"
#include <memory>

namespace svc {
class SceneImp;

class AbstractItem;
using ItemPtr = std::shared_ptr<AbstractItem>;

/**\brief Scene provide 2D infinity space in cartesian koordinate system.
 * Provide functional for append, remove and move Items. Support quires
 * operations, saving and restoring
 */
class Scene {
public:
  Scene() noexcept;
  virtual ~Scene() noexcept;

  /**\brief add Item to scene
   */
  void appendItem(ItemPtr item) noexcept;

  /**\brief remove Item from scene
   *
   * \warning produce undefined behaviour if Item don't associated with the
   * Scene
   */
  void removeItem(ItemPtr item);

  /**\return count of items
   */
  size_t count() const noexcept;

  bool empty() const noexcept;

  /**\brief remove all Items from Scene
   */
  void clear() noexcept;

  /**\return minimal bounding Box for all Item on the Scene. If Scene is empty
   * return invalid Box
   */
  Box bounds() const noexcept;

private:
  SceneImp *imp_;
};
} // namespace svc

// Scene.hpp
/**\file provide object for manage many Items in cartesian space
 *
 * \note Use visitors for manage Items on Scene
 *
 * \note Scene uses cartesian koordinate system
 */

#pragma once

#include "svc/base_geometry_types.hpp"
#include <list>
#include <memory>

namespace svc {
class SceneImp;

class AbstractVisitor;

class AbstractItem;
using ItemPtr = std::shared_ptr<AbstractItem>;

using ItemList = std::list<ItemPtr>;

/**\brief Scene provide 2D infinity space in cartesian koordinate system.
 * Provide functional for append, remove and move Items. Support quires
 * operations, saving and restoring
 */
class Scene {
public:
  enum class SpatialIndex { Intersects, Within };

  Scene() noexcept;
  virtual ~Scene() noexcept;

  /**\brief add Item to scene. If the Item has some children they also will be
   * added to the Scene
   *
   * \note the Item will be removed from its parent (if it exists)
   *
   * \note if the Item associated with some other Scene then it will be removed
   * from it
   */
  void appendItem(ItemPtr &item);

  /**\brief remove Item from scene. If Item has some children they also will be
   * removed from the Scene
   *
   * \note you don't need append Items with parent to Scene. When Item was
   * append to parent, then the Item will be aded to Scene manually (if parent
   * already associated with Scene of when it will be associated with some
   * Scene)
   *
   * \warning produce undefined behaviour if Item don't associated with the
   * Scene
   *
   * \warning produce undefined behaviour if parent of added Item not associated
   * wiht the Scene
   */
  void removeItem(AbstractItem *item);

  /**\brief Item, after change own position, must notificate the Scene about it
   * for update spatial indicies
   *
   * \note item must be associated with the Scene
   */
  void updateItemPosition(AbstractItem *item);

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

  /**\brief spatial query by Point
   */
  ItemList query(Point pos) const noexcept;

  /**\brief spatial query by Box
   */
  ItemList query(Box box, SpatialIndex index = SpatialIndex::Intersects) const
      noexcept;

  /**\brief iterate all main Items (without parents) and call accept method.
   * Highly recomended use visitor for iterate Items
   */
  void accept(AbstractVisitor *visitor);

private:
  SceneImp *imp_;
};
} // namespace svc

// AbstractItem.hpp
/**\file provide base class for Items
 *
 * \note Item uses cartesian koordinate system
 *
 * Every Item has 3 types of koordinates:
 * - item koordinates
 * - parent koordinates
 * - scene koordinates
 *
 * Item koordinates are koordinates, where {0, 0} is a center of the Item. For
 * example: if Item has Scene position as {10, 0} and rotation angle as 90 deg
 * clockwise, then vector from the Item to Scene position {8, 0} in item
 * koordinates will be {0, 2}
 *
 * Parent koordinates are koordinates relatively to parent of the Item. If the
 * Item don't has any parent, then the koordinates will be same to Scene
 * koordinates
 *
 * Scene koordinates are absolute koordinates.
 *
 * \note highly recomended to use visitors for manage Items
 */

#pragma once

#include "svc/base_geometry_types.hpp"
#include <list>
#include <memory>

namespace svc {
class Scene;
class AbstractItemImp;
class AbstractVisitor;

class AbstractItem;
using ItemPtr = std::shared_ptr<AbstractItem>;

/**\brief provide functionality for describing some kind of Item on Scene.
 * Realised as Compositor
 * \todo think about scaling - is this needed?
 */
class AbstractItem {
  friend Scene;

public:
  using Children = std::list<ItemPtr>;

  AbstractItem() noexcept;
  virtual ~AbstractItem() noexcept;

  /**\return bounding box which bounded all shape of the Item. Must be in item
   * koordinates
   *
   * \note all shape of Item must be in the bounding box. It is needed for
   * queries by positions, intersections and collisions
   *
   * \note bounding box never transforms! If you rotate or scale your Item the
   * bounding box will be same
   */
  virtual Box getBoundingBox() const noexcept = 0;

  /**\brief for ierarchy of Item-s must be realized ierarchy of visitors
   *
   * \note hidhly recomended don't add visiting children in the method. It must
   * be a logic of visitior
   *
   * \note AbstractVisitor not a part of basic classes, this is a placeholder.
   * New Item ierarchy must be realize with parallel ierarchy of visitors. You
   * must define your own AbstractVisitor as base of your onw visitors
   */
  virtual void accept(AbstractVisitor *visitor) = 0;

  /**\return Scene with that the Item associated. Return nullptr if Item not
   * associated with any scene
   *
   * \see Scene
   */
  Scene *getScene() const noexcept;

  /**\return position of Item in Scene koordinates
   *
   * \warning produce undefined beheviour if Item not associated with any Scene
   */
  Point getScenePos() const noexcept;

  /**\return current position of Item in parent koordinates (relatively to
   * parent position). If Item don't has any parent then return values will be
   * as Scene kooridnate
   *
   * \see getScenePos
   */
  Point getPos() const noexcept;

  /**\brief move Item on diff relatively to current position
   *
   * \note the operation change Scene position of all child Item-s
   *
   * \param diff vector in Item koordinates
   */
  void moveOn(Point diff);

  /**\brief set position of the Item relatively to Scene
   *
   * \note change Scene position of all child Item-s
   */
  void setScenePos(Point scenePos);

  /**\brief set position of the Item relatively to parent. If the Item don't has
   * a parent then result will be same as set Scene position
   *
   * \note change Scene position of all child Item-s
   *
   * \see setScenePos
   */
  void setPos(Point pos);

  /**\return rotation angle in radians
   */
  float getRotation() const noexcept;

  /**\return rotation angle of the Item relative to Scene
   */
  float getSceneRotation() const noexcept;

  /**\brief rotate current Item
   *
   * \param angle in radians
   *
   * \param anchor in Item koordinates
   */
  void rotate(float angle, Point anchor = {0, 0});

  /**\brief set rotation of the Item relatively to parent
   *
   * \param angle in radians
   *
   * \param anchor in Item koordinates
   */
  void setRotation(float angle, Point anchor = {0, 0});

  /**\brief set rotation angle for the Item relative to Scene
   *
   * \param angle in radians
   *
   * \param achor in Item koordinates
   */
  void setSceneRotation(float angle, Point anchor = {0, 0});

  /**\brief append new child for the Item. If child already have parent, then
   * ownership of the child will be moved from previous parent to the Item. If
   * parent has a Scene, then we child will be add to the Scene
   *
   * \note when child appends to parent then the child don't change own Scene
   * position
   *
   * \warning if you append parent (or parent of parent) of the Item to children
   * it produce undefined behaviour
   */
  void appendChild(ItemPtr child);

  /**\brief remove the child from children of the item.
   *
   * \throw exception if child can not be removed
   *
   * \note it remove the Item from Scene
   */
  void removeChild(AbstractItem *child);

  /**\return list of all leaf nodes of the Item (not includes leaf nodes of
   * children Items)
   *
   * \note hidhly recomended to use visitor for manage children of Item
   */
  Children getChildren() const noexcept;

  /**\return true if the Item don't has any child
   */
  bool empty() const noexcept;

  /**\return count of children (only first level)
   */
  size_t count() const noexcept;

  /**\return parent of the Item or nullptr if Item not have any parent
   */
  AbstractItem *getParent() const noexcept;

  /**\return affine transformation matrix for the Item relatively to parent. If
   * Item not has a parent it will be same as Scene matrix
   *
   * \see getSceneMatrix
   */
  Matrix getMatrix() const noexcept;

  /**\return affine transformation matrix for the Item relatively to Scene
   */
  Matrix getSceneMatrix() const noexcept;

  /**\note translation relative to parent
   */
  void setMatrix(Matrix mat);

private:
  void setScene(Scene *scene) noexcept;

private:
  AbstractItemImp *imp_;

  Scene *scene_;

  AbstractItem *parent_;
  Children      children_;
};
} // namespace svc

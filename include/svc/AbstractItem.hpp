// AbstractItem.hpp
/**\file
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
   * koordinates, so {0, 0} point is same to point returned by getPos function
   *
   * \see getPos
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
   * parent position). If Item not has a parent then return values will be as
   * Scene kooridnate
   *
   * \see getScenePos
   */
  Point getPos() const noexcept;

  /**\brief move Item on diff relatively to current position
   *
   * \note change Scene position of all child Item-s
   *
   * \param diff vector to move
   */
  void moveOn(Point diff) noexcept;

  /**\brief set position of the Item relatively to Scene
   *
   * \note change Scene position of all child Item-s
   */
  void setScenePos(Point scenePos) noexcept;

  /**\brief set position of the Item relatively to parent. If the Item don't has
   * a parent then result will be same as set Scene position
   *
   * \note change Scene position of all child Item-s
   *
   * \see setScenePos
   */
  void setPos(Point pos) noexcept;

  /**\return rotation angle in radians
   *
   * \param anchor in item koordinates
   */
  float getRotation(Point anchor = {0, 0}) const noexcept;

  /**\return rotation angle of the Item relative to Scene
   */
  float getSceneRotation(Point anchor = {0, 0}) const noexcept;

  /**\brief rotate current Item
   *
   * \param angle in radians
   *
   * \param anchor in item koordinates
   */
  void rotate(float angle, Point anchor = {0, 0}) noexcept;

  /**\brief set rotation of the Item relatively to parent
   *
   * \param angle in radians
   *
   * \param anchor in item koordinates
   */
  void setRotation(float angle, Point anchor = {0, 0}) noexcept;

  /**\brief set rotation angle for the Item relative to Scene
   */
  void setSceneRotation(float angle, Point anchor = {0, 0}) noexcept;

  /**\return affine transformation matrix for the Item relatively to parent. If
   * Item not has a parent it will be same as Scene matrix
   *
   * \see getSceneMatrix
   */
  Matrix getMatrix() const noexcept;

  /**\return affine transformation matrix for the Item relatively to Scene
   */
  Matrix getSceneMatrix() const noexcept;

  /**\return parent of the Item or nullptr if Item not have any parent
   */
  AbstractItem *getParent() const noexcept;

  /**\return list of all leaf nodes of the Item (not includes leaf nodes of
   * children Items)
   */
  Children getChildren() const noexcept;

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
  void appendChild(ItemPtr child) noexcept;

  /**\brief remove the child from children of the item.
   *
   * \note it remove the Item from Scene
   *
   * \warning if Item for removing is not a child of the Item it produce
   * undefined behaviour
   */
  void removeChild(ItemPtr child) noexcept;

private:
  void setScene(Scene *scene) noexcept;

private:
  AbstractItemImp *imp_;

  AbstractItem *parent_;
  Children      children_;
};
} // namespace svc

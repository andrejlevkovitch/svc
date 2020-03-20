// AbstractItem.hpp
/**\file
 */

#pragma once

#include "svc/base_geometry_types.hpp"
#include <list>
#include <memory>

namespace svc {
class Scene;

/**\brief provide functionality for describing some kind of Item on Scene.
 * Realised as Compositor
 */
class AbstractItem : public std::enable_shared_from_this<AbstractItem> {
  friend Scene;

public:
  using ChildItemPtr = std::shared_ptr<AbstractItem>;

  AbstractItem() noexcept;
  virtual ~AbstractItem() noexcept = default;

  /**\return Scene with that the Item associated. Return nullptr if Item not
   * associated with any scene
   * \see Scene
   */
  Scene *getScene() const noexcept;

  /**\return current position of Item on Scene
   *
   * \warning produce undefined beheviour if Item not associated with any Scene
   */
  virtual Point getPos() const noexcept;

  /**\return bounding box which bounded all shape of the Item
   */
  virtual Box getBoundingBox() const noexcept = 0;

  /**\return parent of the Item or nullptr if Item not have any parent
   */
  AbstractItem *parent() const noexcept;

  /**\return list of all leaf nodes of the Item (not includes leaf nodes of
   * children Items)
   */
  const std::list<ChildItemPtr> &children() const noexcept;

  /**\brief append new child for the Item. If child already have parent, then
   * ownership of the child will be moved from previous parent to the Item
   *
   * \warning if you append parent (or parent of parent) of the Item to children
   * it produce undefined behaviour
   */
  void appendChild(ChildItemPtr child) noexcept;

  /**\brief remove the child from children of the item
   *
   * \warning if Item for removing is not a child of the Item it produce
   * undefined behaviour
   */
  void removeChild(AbstractItem &child) noexcept;

private:
  void setScene(Scene *scene) noexcept;

private:
  Scene *scene_;

  AbstractItem *          parent_;
  std::list<ChildItemPtr> children_;
};
} // namespace svc

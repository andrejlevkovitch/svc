// AbstractView.hpp
/**\file provide base class for views
 *
 * \note View uses Image koordinate system (rotation on 180 degrees cartesian
 * koordinate system). All transformations between Scene and View koordinate
 * systems are automatical
 */

#pragma once

#include "base_geometry_types.hpp"

namespace svc {
class Scene;
class AbstractVisitor;

class AbstractViewImp;

using ScenePtr = std::shared_ptr<Scene>;

class AbstractView {
public:
  AbstractView() noexcept;
  virtual ~AbstractView() noexcept;

  /**\return a render context size of the View
   */
  virtual Size getContextSize() const noexcept = 0;

  void setScene(ScenePtr scene) noexcept;

  ScenePtr getScene() const noexcept;

  /**\brief set Scene area for View
   *
   * \param sceneRect in Scene koordinates
   */
  void setSceneRect(Rect sceneRect) noexcept;

  /**\return current Scene area (in Scene koordinates)
   *
   * \note by default return rect same to View rect (0,0 as minCorner, viewSize
   * and 0 angle)
   *
   * \see setSceneRect
   */
  Rect getSceneRect() const noexcept;

  /**\param angle in radians
   *
   * \param anchor in View koordinates
   */
  void rotateSceneRect(float angle, Point anchor = {0, 0}) noexcept;

  /**\param vec vector for move current Scene Rect. In View koordinates
   */
  void moveSceneRect(svc::Point vec) noexcept;

  /**\param anchor in View koordinates
   */
  void scaleSceneRect(ScaleFactors factors, Point anchor = {0, 0}) noexcept;

  /**\brief iterate all Items (without hierarchy) in Scene Rect and call accept
   * method for each
   */
  void accept(AbstractVisitor *visitor);

  /**\return transformation matrix for transform points in View koordinates to
   * Scene koordinates
   */
  Matrix getSceneTransformMatrix() const noexcept;

  void setSceneTransformMatrix(Matrix mat) noexcept;

  /**\brief map View point to Scene point
   */
  Point mapToScene(Point viewPoint) const noexcept;

private:
  AbstractViewImp *imp_;

  ScenePtr scene_;
};
} // namespace svc

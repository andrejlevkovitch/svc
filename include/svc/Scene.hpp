// Scene.hpp
/**\file
 */

#pragma once

namespace svc {
class SceneImp;

/**\brief Scene provide 2D infinity space in cartesian koordinate system.
 * Provide functional for append, remove and move Items. Support quires
 * operations, saving and restoring
 */
class Scene {
public:
  Scene() noexcept;
  virtual ~Scene() noexcept;

private:
  SceneImp *imp_;
};
} // namespace svc

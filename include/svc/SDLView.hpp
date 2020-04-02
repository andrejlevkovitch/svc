// SDLView.hpp

#pragma once

#include "svc/AbstractView.hpp"

namespace svc {
class SDLViewImp;

/**\brief provide SDL window with OpenGL context
 * \note uses OpenGL ES 3.0
 */
class SDLView : public AbstractView {
public:
  /**\brief create new window
   * \param screenPos position of new window in screen koordinates
   * \param windowSize size of window in screen koordinates
   * \throw exception if can not create window
   */
  SDLView(std::string_view title, Point screenPos, Size windowSize);

  /**\brief create centered window
   * \throw exception if can not create window
   */
  SDLView(std::string_view title, Size windowSize);

  ~SDLView() override;

  Size getContextSize() const noexcept override;

  /**\brief set window context as current. By default created View is current.
   * If you use only on View you not need use the function
   */
  void makeCurrent();

  /**\brief accept renderer by all Items in View Rect and swap OpenGL context
   */
  void render(AbstractVisitor *renderer);

private:
  SDLViewImp *imp_;
};
} // namespace svc

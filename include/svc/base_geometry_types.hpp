// base_geometry_types.hpp
/**\file contains declaration base geometry types
 */

#pragma once

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/qvm/mat.hpp>
#include <boost/qvm/mat_operations.hpp>
#include <boost/qvm/vec.hpp>
#include <boost/qvm/vec_mat_operations.hpp>
#include <boost/qvm/vec_operations.hpp>
#include <boost/qvm/vec_traits.hpp>

#define TO_RAD(angle) ((angle) * (M_PI / 180.))

#define TWO_PI (M_PI * 2)
/**\brief minimize radians angle, but save sign (so result angle can be
 * negative)
 */
#define MINIMIZE_RADIANS(angle)                                                \
  (((angle) / TWO_PI) - int((angle) / TWO_PI)) * TWO_PI
/**\brief normalize angle in radians
 *
 * \return angle in radians between 0 and 2pi
 */
#define NORM_RADIANS(angle) MINIMIZE_RADIANS(MINIMIZE_RADIANS(angle) + TWO_PI)

namespace bg = boost::geometry;
namespace bq = boost::qvm;

// XXX need for using vector and matrix operations
using namespace bq;

namespace svc {
/**\brief 2D point
 *
 * \note you should set initial values manually, because after creating by
 * default konstructor the variable will contain a garbage
 */
template <typename AritmeticType>
struct Point_ final {
  AritmeticType a[2];

  inline AritmeticType x() const noexcept {
    return a[0];
  }

  inline AritmeticType y() const noexcept {
    return a[1];
  }
};

template <typename AritmeticType>
struct Size_ final {
  AritmeticType a[2];

  inline AritmeticType width() const noexcept {
    return a[0];
  }

  inline AritmeticType height() const noexcept {
    return a[1];
  }

  Size_ operator-(Size_ second) {
    return Size_{a[0] - second.a[0], a[1] - second.a[1]};
  }

  Size_ operator+(Size_ second) {
    return Size_{a[0] + second.a[0], a[1] + second.a[1]};
  }

  Size_ operator/(float n) {
    return Size_{a[0] / n, a[1] / n};
  }

  Size_ operator*(float n) {
    return Size_{a[0] * n, a[1] * n};
  }

  operator Point_<AritmeticType>() const {
    return Point_<AritmeticType>{a[0], a[1]};
  }
};

using Vector = bq::vec<float, 3>;
using Point  = Point_<float>;
using Size   = Size_<float>;
using Box    = bg::model::box<Point>;

/**\brief clockwised (in cartesian koordinate system) convex polygon
 */
using Ring = bg::model::ring<Point, true, false>;

using ScaleFactors = std::pair<float, float>;

/**\brief affine transformation matrix
 */
using Matrix = bq::mat<float, 3, 3>;

inline ScaleFactors getScaleFactors(const Matrix &mat) {
  std::pair<float, float> xVec{mat.a[0][0], mat.a[1][0]};
  std::pair<float, float> yVec{mat.a[0][1], mat.a[1][1]};

  return {std::sqrt(std::pow(xVec.first, 2) + std::pow(xVec.second, 2)),
          std::sqrt(std::pow(yVec.first, 2) + std::pow(yVec.second, 2))};
}

inline float getRotation(const Matrix &mat) {
  auto [xFactor, yFactor] = getScaleFactors(mat);
  float sinAngl           = mat.a[0][1] / yFactor;
  float cosAngl           = mat.a[0][0] / xFactor;
  return std::atan2(-sinAngl, cosAngl);
}

/**\brief similar to Box, but provide rotating
 *
 * \note if you not need rotation the highly recomended to use Box
 */
class Rect final {
public:
  /**\param anchor relative to minCorner
   *
   * \param angle in radians
   */
  Rect(Point minCorner, Size size, float angle, Point anchor = {0, 0});

  /**\brief move the Rect on vector vec
   */
  void  moveOn(Point vec) noexcept;
  void  setMinCorner(Point minCorner) noexcept;
  Point getMinCorner() const noexcept;

  void setSize(Size size) noexcept;
  Size size() const noexcept;

  /**\param anchor relative to minCorner
   *
   * \param angle in radians
   */
  void rotate(float angle, Point anchor = {0, 0}) noexcept;

  /**\param anchor relative to minCorner
   *
   * \param angle in radians
   */
  void setRotation(float angle, Point anchor = {0, 0}) noexcept;

  /**\return angle in radians
   */
  float getRotation() const noexcept;

  void   setMatrix(Matrix mat) noexcept;
  Matrix getMatrix() const noexcept;

  operator Ring() const noexcept;

private:
  Matrix matrix_;
  Size   size_;
};
} // namespace svc

// add point traits for Point_
namespace boost::geometry::traits {
template <typename CoordinateType>
struct tag<svc::Point_<CoordinateType>> {
  using type = point_tag;
};

template <typename CoordinateType>
struct coordinate_type<svc::Point_<CoordinateType>> {
  using type = CoordinateType;
};

template <typename CoordinateType>
struct coordinate_system<svc::Point_<CoordinateType>> {
  using type = cs::cartesian;
};

template <typename CoordinateType>
struct dimension<svc::Point_<CoordinateType>> : boost::mpl::int_<2> {};

template <typename CoordinateType, std::size_t Dimension>
struct access<svc::Point_<CoordinateType>, Dimension> {
  using PointType = svc::Point_<CoordinateType>;

  static inline CoordinateType get(const PointType &point) {
    return point.a[Dimension];
  }

  static inline void set(PointType &point, const CoordinateType &value) {
    point.a[Dimension] = value;
  }
};
} // namespace boost::geometry::traits

// add vector traits for Point_
namespace boost::qvm {
template <typename AritmeticType>
struct vec_traits<svc::Point_<AritmeticType>> {
  using Vector = svc::Point_<AritmeticType>;

  static const int dim = 2;

  using scalar_type = AritmeticType;

  template <int Index>
  static inline scalar_type read_element(const Vector &vec) {
    return vec.a[Index];
  }

  template <int Index>
  static inline scalar_type &write_element(Vector &vec) {
    return vec.a[Index];
  }

  static inline scalar_type read_element_idx(int index, const Vector &vec) {
    return vec.a[index];
  }

  static inline scalar_type &write_element_idx(int index, Vector &vec) {
    return vec.a[index];
  }
};
} // namespace boost::qvm

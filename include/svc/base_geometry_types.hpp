// base_geometry_types.hpp
/**\file contains declaration base geometry types
 */

#pragma once

#include <boost/geometry/geometries/box.hpp>
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
struct Point_ {
  AritmeticType a[2];

  inline float x() const {
    return a[0];
  }

  inline float y() const {
    return a[1];
  }
};

template <typename AritmeticType>
using Box_ = bg::model::box<Point_<AritmeticType>>;

using Vector = bq::vec<float, 3>;
using Point  = Point_<float>;
using Box    = Box_<float>;

/**\brief affine transformation matrix
 */
using Matrix = bq::mat<float, 3, 3>;
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

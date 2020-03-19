// base_geometry_types.hpp
/**\file contains declaration base geometry types
 */

#pragma once

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

namespace svc {
namespace bg = boost::geometry;

template <typename Aritmetic>
using Point_ = bg::model::d2::point_xy<Aritmetic>;

template <typename Aritmetic>
using Box_ = bg::model::box<Point_<Aritmetic>>;

using Point = Point_<float>;
using Box   = Box_<float>;
} // namespace svc

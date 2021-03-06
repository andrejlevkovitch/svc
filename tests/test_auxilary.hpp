// test_auxilary.hpp
// contains test utils

#pragma once

#include "svc/base_geometry_types.hpp"

#define FIRST_LEVEL_GENERATOR  2
#define SECOND_LEVEL_GENERATOR 2
#define THIRD_LEVEL_GENERATOR  1

// XXX not use 360 - because in this case we has a problem wiht normalizing
// angle
#define ANGLE_GENERATOR(NUM) TO_RAD(GENERATE(take(NUM, random(-350, 350))))

#define POINT_GENERATOR(NUM)                                                   \
  svc::Point {                                                                 \
    float(GENERATE(take(NUM, random(-1000, 1000)))),                           \
        float(GENERATE(take(NUM, random(-1000, 1000))))                        \
  }

#define SIZE_GENERATOR(NUM)                                                    \
  svc::Size {                                                                  \
    float(GENERATE(take(NUM, random(10, 1000)))),                              \
        float(GENERATE(take(NUM, random(10, 1000))))                           \
  }

// the check about 0 is needed becuase epsilon is relative to variable. So if
// variable is 0, then epsilon also is 0
#define CHECK_POINTS_EQUAL(first, second)                                      \
  if (!((int)(first).x() == (int)(second).x() && (int)(second).x() == 0)) {    \
    CHECK(Approx{(first).x()}.epsilon(0.01) == (second).x());                  \
  }                                                                            \
  if (!((int)(first).y() == (int)(second).y() && (int)(second).y() == 0)) {    \
    CHECK(Approx{(first).y()}.epsilon(0.01) == (second).y());                  \
  }

#define CHECK_SIZES_EQUAL(first, second)                                       \
  if (!((int)(first).width() == (int)(second).width() &&                       \
        (int)(second).width() == 0)) {                                         \
    CHECK(Approx{(first).width()}.epsilon(0.01) == (second).width());          \
  }                                                                            \
  if (!((int)(first).height() == (int)(second).height() &&                     \
        (int)(second).height() == 0)) {                                        \
    CHECK(Approx{(first).height()}.epsilon(0.01) == (second).height());        \
  }

#define CHECK_ANGLES_EQUAL(first, second)                                      \
  CHECK(Approx{NORM_RADIANS(first)}.epsilon(0.01) == NORM_RADIANS(second));

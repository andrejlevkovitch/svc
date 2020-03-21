# cmake

cmake_minimum_required(VERSION 3.12)

project(svc)

include(cmake/build.cmake)
include(cmake/doxygen.cmake)
include(cmake/catch2_test_register.cmake)

add_subdirectory(third-party)

find_package(Boost REQUIRED)

set(SVC_SRC
  src/svc/Scene.cpp
  src/svc/AbstractItem.cpp
  )

add_library(${PROJECT_NAME} ${SVC_SRC})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PRIVATE
  Boost::boost
  )


# add tests
catch2_test_register(test_scene     tests/test_Scene.cpp)
catch2_test_register(test_item      tests/test_Item.cpp)

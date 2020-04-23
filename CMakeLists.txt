# cmake

cmake_minimum_required(VERSION 3.12)

project(svc)

include(cmake/build.cmake)
include(cmake/doxygen.cmake)
include(cmake/catch2_test_register.cmake)
include(cmake/example_register.cmake)

add_subdirectory(third-party)

find_package(Boost COMPONENTS serialization REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

set(SVC_SRC
  src/svc/Rect.cpp

  src/svc/Scene.cpp
  src/svc/AbstractItem.cpp
  src/svc/AbstractView.cpp

  src/svc/SDLView.cpp
  src/svc/SDLController.cpp

  src/gl/misc.cpp
  )

add_library(${PROJECT_NAME} ${SVC_SRC})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} INTERFACE
  Boost::boost
  ${Boost_LIBRARIES}
  SDL2::SDL2
  OpenGL::GL
  )
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
  target_compile_options(${PROJECT_NAME} PUBLIC -DCOLORIZED)
endif()


# add tests
catch2_test_register(test_scene         tests/test_Scene.cpp)
catch2_test_register(test_item          tests/test_Item.cpp)
catch2_test_register(test_rect          tests/test_Rect.cpp)
catch2_test_register(test_view          tests/test_View.cpp)
catch2_test_register(test_serialization tests/test_serialization.cpp)


example_register(sdl_view               examples/sdl_view.cpp)

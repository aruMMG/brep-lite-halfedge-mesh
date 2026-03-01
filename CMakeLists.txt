cmake_minimum_required(VERSION 3.20)
project(brep_lite_halfedge_mesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(brep_lite INTERFACE)
target_include_directories(brep_lite INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(cube_to_obj apps/cube_to_obj.cpp)
target_link_libraries(cube_to_obj PRIVATE brep_lite)

add_executable(test_cube tests/test_cube.cpp)
target_link_libraries(test_cube PRIVATE brep_lite)

enable_testing()
add_test(NAME cube_tests COMMAND test_cube)
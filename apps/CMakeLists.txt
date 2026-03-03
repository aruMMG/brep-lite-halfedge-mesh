# apps/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

add_executable(halfedge_viewer
  ${CMAKE_CURRENT_LIST_DIR}/viewer/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/viewer/mesh_renderer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/viewer/obj_loader.cpp
)

target_link_libraries(halfedge_viewer PRIVATE imgui brep_lite)

# If you want to include your headers later:
# target_include_directories(halfedge_viewer PRIVATE ${CMAKE_SOURCE_DIR}/include)

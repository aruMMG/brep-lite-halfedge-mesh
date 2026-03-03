# external/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# ---- GLFW ----
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# ---- OpenGL ----
find_package(OpenGL REQUIRED)

# ---- GLEW (OpenGL function loader) ----
find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GLEW REQUIRED glew)
endif()

add_library(glew_iface INTERFACE)
target_include_directories(glew_iface INTERFACE ${GLEW_INCLUDE_DIRS})
target_link_libraries(glew_iface INTERFACE ${GLEW_LIBRARIES})

# ---- Dear ImGui ----
add_library(imgui STATIC
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_tables.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp

  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_glfw.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_opengl3.cpp

)

target_include_directories(imgui PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/imgui
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends
)

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL glew_iface)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Some Linux setups need dl for GL function loading
if(UNIX AND NOT APPLE)
  target_link_libraries(imgui PUBLIC dl)
endif()
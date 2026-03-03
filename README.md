# brep-lite-halfedge-mesh

A lightweight C++20 half-edge mesh playground focused on **core topology operations** and a small rendering demo.

## What this project is

This repository implements a compact half-edge data structure suitable for learning and prototyping:

- half-edge/vertex/face storage with ID handles
- polygon face insertion with automatic twin linking
- topology traversals:
  - face boundary walk
  - vertex one-ring walk
- mesh validation checks:
  - local pointer consistency (`next/prev/twin`)
  - face loop closure checks
  - closed-manifold edge count checks
  - optional Euler characteristic check
- OBJ export utility
- basic test coverage for cube construction/traversal/validation
- OpenGL + ImGui viewer target

---

## Repository layout

- `include/brep_lite/halfedge_mesh.hpp` – core half-edge mesh implementation.
- `include/brep_lite/cube_builder.hpp` – procedural quad-cube builder.
- `tests/test_cube.cpp` – small standalone test executable.
- `apps/cube_to_obj.cpp` – CLI tool that builds/validates cube and writes OBJ.
- `apps/viewer/*` – GLFW/GLEW/ImGui viewer and mesh renderer.
- `external/` – third-party setup (ImGui, GLFW, GLEW via CMake config in this repo).

---

## Build

### Prerequisites

- CMake 3.20+
- C++20 compiler (Clang/GCC/MSVC)
- OpenGL-capable environment for viewer target

### Configure and compile

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

---

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

Or run the test binary directly:

```bash
./build/test_cube
```

---

## CLI: export a cube to OBJ

```bash
# defaults: output=cube.obj, half_extent=1.0
./build/cube_to_obj

# custom output and size
./build/cube_to_obj out/cube.obj 0.5
```

---

## Viewer

After building:

```bash
./build/apps/halfedge_viewer
```

The viewer currently builds a test cube, validates it, writes a temporary OBJ, reloads it, and renders it.

---

## Design notes / current limitations

- `vertex_one_ring` assumes a closed manifold around the selected vertex.
- `add_face` is intentionally minimal and does not prevent all invalid authoring patterns by itself.
- Validation is robust for local consistency and manifold-edge checks, but this is not yet a full CAD-grade topological kernel.
- `apps/viewer/mesh_extract_user.hpp` still contains a fallback mesh extraction stub and TODO comments.

---

## Quick review opinion

This is a strong educational foundation:

- The core half-edge implementation is compact and readable.
- The validator catches common topology bugs early.
- The cube builder + tests make behavior easy to verify.

Main opportunities to improve next:

1. Complete real half-edge-to-triangle extraction in the viewer path (remove fallback stub).
2. Add explicit boundary mesh support (or strict rejection rules) in traversal APIs.
3. Expand tests beyond the cube (non-convex polygons, open meshes, malformed inputs).
4. Separate topology logic from IO/utility concerns if aiming for a reusable library API.

If your goal is “lightweight but trustworthy topology infrastructure,” this project is on the right track.

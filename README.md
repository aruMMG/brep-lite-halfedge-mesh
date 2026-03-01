# BREP-lite Half-Edge Mesh (C++20)

Minimal half-edge mesh with:
- Procedural cube (quads)
- Face boundary + vertex one-ring traversals
- Validators (local consistency, closed-manifold, optional Euler check)
- OBJ exporter (v + f)
- Small test suite

## Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
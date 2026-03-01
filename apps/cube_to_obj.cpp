#include <iostream>
#include <filesystem>
#include <string>
#include <cstdlib>   // std::strtod
#include "brep_lite/cube_builder.hpp"

static double parse_double(const char* s, double fallback) {
  if (!s) return fallback;
  char* end = nullptr;
  double v = std::strtod(s, &end);
  if (end == s || *end != '\0') return fallback;
  return v;
}

int main(int argc, char** argv) {
  // Usage:
  //   cube_to_obj [output.obj] [half_extent]
  // Defaults:
  //   output.obj = cube.obj
  //   half_extent = 1.0  (edge length = 2.0)
  std::filesystem::path out = (argc > 1) ? argv[1] : "cube.obj";
  double half_extent = (argc > 2) ? parse_double(argv[2], 1.0) : 1.0;

  if (half_extent <= 0.0) {
    std::cerr << "half_extent must be > 0\n";
    return 1;
  }

  brep_lite::HalfEdgeMesh m = brep_lite::build_cube_quads(half_extent);

  auto errs = m.validate(true);
  if (!errs.empty()) {
    std::cerr << "Mesh validation failed:\n";
    for (auto& e : errs) std::cerr << "  - " << e << "\n";
    return 1;
  }

  if (!m.write_obj(out)) {
    std::cerr << "Failed to write OBJ: " << out << "\n";
    return 1;
  }

  std::cout << "Wrote " << out << " (half_extent=" << half_extent
            << ", edge_length=" << (2.0 * half_extent) << ")\n";
  return 0;
}
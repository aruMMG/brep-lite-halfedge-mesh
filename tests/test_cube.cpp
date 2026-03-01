#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "brep_lite/cube_builder.hpp"

namespace {

using TestFn = void(*)();
struct Test { const char* name; TestFn fn; };
inline std::vector<Test>& registry() { static std::vector<Test> r; return r; }

struct Registrar {
  Registrar(const char* name, TestFn fn) { registry().push_back({name, fn}); }
};

#define TEST(name) \
  void name(); \
  static Registrar reg_##name(#name, &name); \
  void name()

#define REQUIRE(cond) do { \
  if (!(cond)) throw std::runtime_error(std::string("REQUIRE failed: ") + #cond); \
} while(0)

static bool eq_exact(const std::vector<brep_lite::VertexID>& a,
                     const std::vector<uint32_t>& b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) if (a[i].idx != b[i]) return false;
  return true;
}

} // namespace

TEST(Cube_Validate_NoErrors) {
  auto m = brep_lite::build_cube_quads();
  auto errs = m.validate(true);
  REQUIRE(errs.empty());
  REQUIRE(m.vertex_count() == 8);
  REQUIRE(m.face_count() == 6);
  REQUIRE(m.halfedge_count() == 24); // 6 quads * 4 = 24
}

TEST(Cube_FaceBoundary_Top) {
  auto m = brep_lite::build_cube_quads();
  // top is face 1 in builder
  auto loop = m.face_boundary(brep_lite::FaceID{1});
  REQUIRE(eq_exact(loop, {4,5,6,7}));
}

TEST(Cube_VertexOneRing_V0) {
  auto m = brep_lite::build_cube_quads();
  auto ring = m.vertex_one_ring(brep_lite::VertexID{0});
  // With our build order + vertex.out assignment, expected order is [3,4,1]
  REQUIRE(ring.size() == 3);
  REQUIRE(ring[0].idx == 3);
  REQUIRE(ring[1].idx == 4);
  REQUIRE(ring[2].idx == 1);
}

TEST(Cube_Validator_CatchesBrokenTwin) {
  brep_lite::HalfEdgeMesh m = brep_lite::build_cube_quads();

  // Smash by re-building a non-manifold edge: easiest is add a duplicate face
  // (this should cause undirected edge counts != 2 in validate()).
  // Duplicate the top face:
  m.add_face({ brep_lite::VertexID{4}, brep_lite::VertexID{5}, brep_lite::VertexID{6}, brep_lite::VertexID{7} });

  auto errs = m.validate(false);
  REQUIRE(!errs.empty());
  bool found = false;
  for (auto& e : errs) {
    if (e.find("Non-manifold") != std::string::npos) { found = true; break; }
  }
  REQUIRE(found);
}

int main() {
  int passed = 0;
  for (const auto& t : registry()) {
    try {
      t.fn();
      ++passed;
    } catch (const std::exception& e) {
      std::cerr << "[FAIL] " << t.name << ": " << e.what() << "\n";
      return 1;
    }
  }
  std::cout << "[OK] " << passed << " tests passed\n";
  return 0;
}
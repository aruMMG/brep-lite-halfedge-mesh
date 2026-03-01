#pragma once
#include "halfedge_mesh.hpp"
#include <vector>

namespace brep_lite {

// half_extent = 1 -> cube from -1..+1 on each axis (edge length 2)
// half_extent = 0.5 -> edge length 1
inline HalfEdgeMesh build_cube_quads(double half_extent = 1.0) {
  HalfEdgeMesh m;

  const double h = half_extent;
  const std::vector<Vec3> P = {
    {-h,-h,-h}, { h,-h,-h}, { h, h,-h}, {-h, h,-h},
    {-h,-h, h}, { h,-h, h}, { h, h, h}, {-h, h, h}
  };

  std::vector<VertexID> v(8);
  for (int i = 0; i < 8; ++i) v[i] = m.add_vertex(P[i]);

  m.add_face({ v[0], v[3], v[2], v[1] }); // bottom (-Z)
  m.add_face({ v[4], v[5], v[6], v[7] }); // top (+Z)
  m.add_face({ v[0], v[1], v[5], v[4] }); // front (-Y)
  m.add_face({ v[3], v[7], v[6], v[2] }); // back (+Y)
  m.add_face({ v[0], v[4], v[7], v[3] }); // left (-X)
  m.add_face({ v[1], v[2], v[6], v[5] }); // right (+X)

  return m;
}

} // namespace brep_lite
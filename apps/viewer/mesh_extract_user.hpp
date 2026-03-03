#pragma once
#include "mesh_data.hpp"

// Your core:
#include "brep_lite/halfedge_mesh.hpp"
#include "brep_lite/cube_builder.hpp"

namespace viewer {

// IMPORTANT:
// Implement this by reusing the triangulation logic you already wrote in
// apps/cube_to_obj.cpp.
// Instead of writing OBJ, fill MeshData.positions + MeshData.indices.
//
// positions: push x,y,z for each unique vertex (same order as you assign indices)
// indices: triangles as triplets (0-based).
inline MeshData ExtractMeshDataFromHalfEdge(const brep_lite::HalfEdgeMesh& /*mesh*/)
{
    MeshData out;

    // --- TEMP FALLBACK (so you can compile immediately) ---
    // Replace with your real extraction ASAP.
    // Simple cube (12 triangles, 8 vertices):
    const float p[] = {
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f
    };
    out.positions.assign(p, p + 8*3);

    const uint32_t idx[] = {
        0,1,2,  2,3,0, // back
        4,6,5,  6,4,7, // front
        0,4,5,  5,1,0, // bottom
        3,2,6,  6,7,3, // top
        0,3,7,  7,4,0, // left
        1,5,6,  6,2,1  // right
    };
    out.indices.assign(idx, idx + 12*3);

    return out;
}

// Build a mesh using your cube builder.
// If your API name differs, change ONLY this function body.
inline brep_lite::HalfEdgeMesh BuildTestMesh()
{
    // Common patterns; pick the one matching your project:
    // return brep_lite::build_cube();
    // return brep_lite::CubeBuilder::Build();
    // return brep_lite::make_cube();
    return brep_lite::build_cube_quads(); // <-- adjust to your cube_builder.hpp
}

} // namespace viewer
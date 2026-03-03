#pragma once
#include <cstdint>
#include <vector>

struct MeshData {
    // xyzxyz...
    std::vector<float> positions;
    // 3 indices per triangle
    std::vector<uint32_t> indices;
};
#pragma once
#include <filesystem>
#include "mesh_data.hpp"

namespace viewer {

// Loads only:
//   v x y z
//   f a b c [d ...]   (supports v, v/t, v//n, v/t/n; triangulates n-gons via fan)
bool LoadObjMesh(const std::filesystem::path& path, MeshData& out, std::string& err);

} // namespace viewer
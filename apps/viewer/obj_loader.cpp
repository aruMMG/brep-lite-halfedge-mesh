#include "obj_loader.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

namespace viewer {

static inline bool starts_with(const std::string& s, const char* p) {
    return s.rfind(p, 0) == 0;
}

static int parse_obj_index(const std::string& token, int vertex_count, std::string& err) {
    // token formats: "v", "v/t", "v//n", "v/t/n"
    // we only care about the first component (v)
    std::string vpart = token;
    auto slash = vpart.find('/');
    if (slash != std::string::npos) vpart = vpart.substr(0, slash);

    if (vpart.empty()) {
        err = "OBJ face token has empty vertex index: '" + token + "'";
        return -1;
    }

    int idx = 0;
    try {
        idx = std::stoi(vpart);
    } catch (...) {
        err = "OBJ face token has invalid vertex index: '" + token + "'";
        return -1;
    }

    // OBJ indices are 1-based; negative are relative to end.
    if (idx > 0) {
        idx = idx - 1;
    } else if (idx < 0) {
        idx = vertex_count + idx;
    } else {
        err = "OBJ vertex index cannot be 0: '" + token + "'";
        return -1;
    }

    if (idx < 0 || idx >= vertex_count) {
        err = "OBJ vertex index out of range: '" + token + "'";
        return -1;
    }
    return idx;
}

bool LoadObjMesh(const std::filesystem::path& path, MeshData& out, std::string& err) {
    out.positions.clear();
    out.indices.clear();
    err.clear();

    std::ifstream in(path);
    if (!in) {
        err = "Failed to open OBJ: " + path.string();
        return false;
    }

    std::vector<float> verts; // xyz...
    std::string line;
    int line_no = 0;

    while (std::getline(in, line)) {
        ++line_no;
        if (line.empty()) continue;

        // Trim leading spaces
        size_t i = 0;
        while (i < line.size() && std::isspace((unsigned char)line[i])) ++i;
        if (i == line.size()) continue;
        if (line[i] == '#') continue;

        std::string_view sv(line.c_str() + i, line.size() - i);

        if (sv.size() >= 2 && sv[0] == 'v' && std::isspace((unsigned char)sv[1])) {
            std::istringstream ss{std::string(sv)};
            char v;
            float x, y, z;
            ss >> v >> x >> y >> z;
            if (!ss) {
                err = "Bad vertex line at " + std::to_string(line_no);
                return false;
            }
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);
        }
        else if (sv.size() >= 2 && sv[0] == 'f' && std::isspace((unsigned char)sv[1])) {
            std::istringstream ss{std::string(sv)};
            char f;
            ss >> f;

            std::vector<int> face;
            std::string tok;

            int vcount = (int)(verts.size() / 3);
            while (ss >> tok) {
                std::string perr;
                int vi = parse_obj_index(tok, vcount, perr);
                if (vi < 0) {
                    err = "Line " + std::to_string(line_no) + ": " + perr;
                    return false;
                }
                face.push_back(vi);
            }

            if (face.size() < 3) {
                err = "Line " + std::to_string(line_no) + ": face has < 3 vertices";
                return false;
            }

            // Triangulate fan: (0, i, i+1)
            for (size_t k = 1; k + 1 < face.size(); ++k) {
                out.indices.push_back((uint32_t)face[0]);
                out.indices.push_back((uint32_t)face[k]);
                out.indices.push_back((uint32_t)face[k + 1]);
            }
        }
        // ignore everything else (vn, vt, usemtl, etc.)
    }

    out.positions = std::move(verts);

    if (out.positions.empty() || out.indices.empty()) {
        err = "OBJ had no vertices or faces: " + path.string();
        return false;
    }
    return true;
}

} // namespace viewer
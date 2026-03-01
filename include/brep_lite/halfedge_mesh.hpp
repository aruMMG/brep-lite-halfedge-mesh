#pragma once
#include <cstdint>
#include <limits>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace brep_lite {

constexpr uint32_t kInvalid = std::numeric_limits<uint32_t>::max();

struct VertexID { uint32_t idx = kInvalid; };
struct HalfEdgeID { uint32_t idx = kInvalid; };
struct FaceID { uint32_t idx = kInvalid; };

inline bool valid(VertexID v) { return v.idx != kInvalid; }
inline bool valid(HalfEdgeID h) { return h.idx != kInvalid; }
inline bool valid(FaceID f) { return f.idx != kInvalid; }

inline bool operator==(VertexID a, VertexID b){ return a.idx==b.idx; }
inline bool operator==(HalfEdgeID a, HalfEdgeID b){ return a.idx==b.idx; }
inline bool operator==(FaceID a, FaceID b){ return a.idx==b.idx; }

struct Vec3 {
  double x=0, y=0, z=0;
};

struct Vertex {
  Vec3 pos{};
  HalfEdgeID out{}; // one outgoing half-edge (origin == this vertex)
};

struct Face {
  HalfEdgeID edge{}; // one boundary half-edge on this face
};

struct HalfEdge {
  VertexID origin{};
  HalfEdgeID next{}, prev{}, twin{};
  FaceID face{};
};

class HalfEdgeMesh {
public:
  VertexID add_vertex(const Vec3& p) {
    VertexID id{static_cast<uint32_t>(vertices_.size())};
    vertices_.push_back(Vertex{p, HalfEdgeID{}});
    return id;
  }

  // Add a polygon face given vertex IDs in CCW order (as seen from outside).
  // For cube: pass 4 verts (quads). Minimum 3.
  FaceID add_face(const std::vector<VertexID>& loop) {
    FaceID f{static_cast<uint32_t>(faces_.size())};
    faces_.push_back(Face{HalfEdgeID{}});

    if (loop.size() < 3) {
      // leave invalid; validator will catch
      return f;
    }

    const uint32_t n = static_cast<uint32_t>(loop.size());
    const uint32_t he_base = static_cast<uint32_t>(halfedges_.size());
    halfedges_.resize(halfedges_.size() + n);

    // Create half-edges
    for (uint32_t i = 0; i < n; ++i) {
      HalfEdgeID hid{he_base + i};
      const VertexID a = loop[i];
      const VertexID b = loop[(i + 1) % n];

      HalfEdge& he = halfedges_[hid.idx];
      he.origin = a;
      he.face = f;

      he.next = HalfEdgeID{he_base + ((i + 1) % n)};
      he.prev = HalfEdgeID{he_base + ((i + n - 1) % n)};
      he.twin = HalfEdgeID{}; // set later via map

      // Set face anchor edge
      if (!valid(faces_[f.idx].edge)) faces_[f.idx].edge = hid;

      // Set one outgoing half-edge for vertex if unset
      if (valid(a) && !valid(vertices_[a.idx].out)) vertices_[a.idx].out = hid;

      // Twin linking via directed edge map
      const uint64_t key_ab = pack_key(a, b);
      const uint64_t key_ba = pack_key(b, a);

      auto it = directed_edge_map_.find(key_ba);
      if (it != directed_edge_map_.end()) {
        HalfEdgeID twin_id = it->second;
        halfedges_[hid.idx].twin = twin_id;
        halfedges_[twin_id.idx].twin = hid;
      }

      // Insert after linking attempt (so later faces can find it)
      directed_edge_map_[key_ab] = hid;
    }

    return f;
  }

  // -------- Traversals --------

  // Returns ordered boundary vertex IDs of a face (one full loop).
  std::vector<VertexID> face_boundary(FaceID f) const {
    std::vector<VertexID> out;
    if (!valid(f) || f.idx >= faces_.size()) return out;
    HalfEdgeID start = faces_[f.idx].edge;
    if (!valid(start) || start.idx >= halfedges_.size()) return out;

    HalfEdgeID h = start;
    const size_t max_steps = halfedges_.size() + 1;
    for (size_t step = 0; step < max_steps; ++step) {
      out.push_back(halfedges_[h.idx].origin);
      HalfEdgeID nxt = halfedges_[h.idx].next;
      if (!valid(nxt) || nxt.idx >= halfedges_.size()) break;
      h = nxt;
      if (h == start) break;
    }
    return out;
  }

  // Returns adjacent vertices around v in (typically) CCW order.
  // Requires closed manifold around v (no boundary).
  std::vector<VertexID> vertex_one_ring(VertexID v) const {
    std::vector<VertexID> ring;
    if (!valid(v) || v.idx >= vertices_.size()) return ring;

    HalfEdgeID start = vertices_[v.idx].out;
    if (!valid(start) || start.idx >= halfedges_.size()) return ring;

    HalfEdgeID h = start;
    const size_t max_steps = halfedges_.size() + 1;
    for (size_t step = 0; step < max_steps; ++step) {
      VertexID d = dest(h);
      if (!valid(d)) break;
      ring.push_back(d);

      HalfEdgeID t = halfedges_[h.idx].twin;
      if (!valid(t) || t.idx >= halfedges_.size()) break;

      HalfEdgeID next_around = halfedges_[t.idx].next;
      if (!valid(next_around) || next_around.idx >= halfedges_.size()) break;

      h = next_around;
      if (h == start) break;
    }
    return ring;
  }

  // -------- Validation --------

  std::vector<std::string> validate(bool check_euler = true) const {
    std::vector<std::string> errs;

    auto he_ok = [&](HalfEdgeID h){ return valid(h) && h.idx < halfedges_.size(); };
    auto v_ok  = [&](VertexID v){ return valid(v) && v.idx < vertices_.size(); };
    auto f_ok  = [&](FaceID f){ return valid(f) && f.idx < faces_.size(); };

    // Half-edge field validity + local consistency
    for (uint32_t i = 0; i < halfedges_.size(); ++i) {
      HalfEdgeID hid{i};
      const HalfEdge& he = halfedges_[i];

      if (!v_ok(he.origin)) errs.push_back("HE[" + std::to_string(i) + "]: invalid origin");
      if (!he_ok(he.next))  errs.push_back("HE[" + std::to_string(i) + "]: invalid next");
      if (!he_ok(he.prev))  errs.push_back("HE[" + std::to_string(i) + "]: invalid prev");
      if (!he_ok(he.twin))  errs.push_back("HE[" + std::to_string(i) + "]: invalid twin");
      if (!f_ok(he.face))   errs.push_back("HE[" + std::to_string(i) + "]: invalid face");

      if (he_ok(he.next) && halfedges_[he.next.idx].prev.idx != i)
        errs.push_back("HE[" + std::to_string(i) + "]: next->prev mismatch");

      if (he_ok(he.prev) && halfedges_[he.prev.idx].next.idx != i)
        errs.push_back("HE[" + std::to_string(i) + "]: prev->next mismatch");

      if (he_ok(he.twin) && halfedges_[he.twin.idx].twin.idx != i)
        errs.push_back("HE[" + std::to_string(i) + "]: twin symmetry broken");

      // dest / twin destination consistency
      if (he_ok(he.next) && v_ok(he.origin)) {
        VertexID d = dest(hid);
        if (!v_ok(d)) errs.push_back("HE[" + std::to_string(i) + "]: invalid dest");
      }
      if (he_ok(he.twin) && he_ok(he.next) && he_ok(halfedges_[he.twin.idx].next)) {
        VertexID a = he.origin;
        VertexID b = dest(hid);
        VertexID twin_dest = dest(he.twin);
        if (valid(b) && twin_dest.idx != a.idx)
          errs.push_back("HE[" + std::to_string(i) + "]: twin dest != origin");
      }
    }

    // Face loop closure
    for (uint32_t fi = 0; fi < faces_.size(); ++fi) {
      FaceID f{fi};
      HalfEdgeID start = faces_[fi].edge;
      if (!he_ok(start)) {
        errs.push_back("Face[" + std::to_string(fi) + "]: invalid edge");
        continue;
      }
      HalfEdgeID h = start;
      const size_t max_steps = halfedges_.size() + 1;
      size_t steps = 0;
      for (; steps < max_steps; ++steps) {
        if (halfedges_[h.idx].face.idx != fi) {
          errs.push_back("Face[" + std::to_string(fi) + "]: half-edge face mismatch");
          break;
        }
        HalfEdgeID nxt = halfedges_[h.idx].next;
        if (!he_ok(nxt)) {
          errs.push_back("Face[" + std::to_string(fi) + "]: broken next");
          break;
        }
        h = nxt;
        if (h == start) break;
      }
      if (steps >= max_steps) errs.push_back("Face[" + std::to_string(fi) + "]: loop did not close");
      if (steps < 2) errs.push_back("Face[" + std::to_string(fi) + "]: loop too small (<3)");
    }

    // Closed-manifold undirected edge count check
    // If good, E = halfedges/2.
    std::unordered_map<uint64_t, uint32_t> undirected_counts;
    undirected_counts.reserve(halfedges_.size());

    for (uint32_t i = 0; i < halfedges_.size(); ++i) {
      HalfEdgeID h{i};
      VertexID a = halfedges_[i].origin;
      VertexID b = dest(h);
      if (!valid(a) || !valid(b)) continue;
      uint64_t key = undirected_key(a, b);
      undirected_counts[key] += 1;
    }

    bool closed_manifold = true;
    for (const auto& [k, c] : undirected_counts) {
      if (c != 2) {
        closed_manifold = false;
        errs.push_back("Non-manifold or boundary edge: undirected edge has count=" + std::to_string(c));
      }
    }

    if (check_euler && closed_manifold && (halfedges_.size() % 2 == 0)) {
      const int V = static_cast<int>(vertices_.size());
      const int E = static_cast<int>(halfedges_.size() / 2);
      const int F = static_cast<int>(faces_.size());
      const int chi = V - E + F;
      if (chi != 2) {
        errs.push_back("Euler characteristic check failed: V-E+F = " + std::to_string(chi) + " (expected 2)");
      }
    }

    return errs;
  }

  // -------- Export --------

  bool write_obj(const std::filesystem::path& path) const {
    std::ofstream os(path);
    if (!os) return false;

    for (const auto& v : vertices_) {
      os << "v " << v.pos.x << " " << v.pos.y << " " << v.pos.z << "\n";
    }

    for (uint32_t fi = 0; fi < faces_.size(); ++fi) {
      std::vector<VertexID> loop = face_boundary(FaceID{fi});
      if (loop.size() < 3) continue;
      os << "f";
      for (VertexID vid : loop) {
        // OBJ is 1-based
        os << " " << (vid.idx + 1);
      }
      os << "\n";
    }
    return true;
  }

  // -------- Introspection --------
  size_t vertex_count() const { return vertices_.size(); }
  size_t halfedge_count() const { return halfedges_.size(); }
  size_t face_count() const { return faces_.size(); }

private:
  // dest(h) = origin(next(h))
  VertexID dest(HalfEdgeID h) const {
    if (!valid(h) || h.idx >= halfedges_.size()) return VertexID{};
    HalfEdgeID nxt = halfedges_[h.idx].next;
    if (!valid(nxt) || nxt.idx >= halfedges_.size()) return VertexID{};
    return halfedges_[nxt.idx].origin;
  }

  static uint64_t pack_key(VertexID a, VertexID b) {
    return (static_cast<uint64_t>(a.idx) << 32) | static_cast<uint64_t>(b.idx);
  }
  static uint64_t undirected_key(VertexID a, VertexID b) {
    uint32_t lo = std::min(a.idx, b.idx);
    uint32_t hi = std::max(a.idx, b.idx);
    return (static_cast<uint64_t>(lo) << 32) | static_cast<uint64_t>(hi);
  }

  std::vector<Vertex> vertices_;
  std::vector<HalfEdge> halfedges_;
  std::vector<Face> faces_;

  // For twin linking during construction
  std::unordered_map<uint64_t, HalfEdgeID> directed_edge_map_;
};

} // namespace brep_lite
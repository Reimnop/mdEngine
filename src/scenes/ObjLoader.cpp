#include "ObjLoader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>

using mdEngine::Vec2;
using mdEngine::Vec3;

namespace
{
  struct Corner
  {
    int v = -1, t = -1, n = -1; // -1 when absent
  };

  // OBJ indices are 1-based, negative ones count back from the end
  int resolveIndex(const long index, const size_t count)
  {
    const long resolved = index > 0 ? index - 1 : static_cast<long>(count) + index;
    if (index == 0 || resolved < 0 || resolved >= static_cast<long>(count))
      throw std::runtime_error("index out of range");
    return static_cast<int>(resolved);
  }

  // v, v/t, v//n or v/t/n
  Corner parseCorner(const std::string& token, const size_t vCount, const size_t tCount, const size_t nCount)
  {
    Corner c;
    const char* p = token.c_str();
    char* end = nullptr;

    c.v = resolveIndex(std::strtol(p, &end, 10), vCount);
    p = end;
    if (*p == '/')
    {
      p++;
      if (*p != '/')
      {
        c.t = resolveIndex(std::strtol(p, &end, 10), tCount);
        p = end;
      }
      if (*p == '/')
        c.n = resolveIndex(std::strtol(p + 1, &end, 10), nCount);
    }
    return c;
  }

  void fitToUnitCube(MeshData& m)
  {
    Vec3 lo = m.v[0].pos, hi = lo;
    for (const auto& v : m.v)
    {
      lo = {std::min(lo.x, v.pos.x), std::min(lo.y, v.pos.y), std::min(lo.z, v.pos.z)};
      hi = {std::max(hi.x, v.pos.x), std::max(hi.y, v.pos.y), std::max(hi.z, v.pos.z)};
    }

    const Vec3 size = hi - lo;
    const float extent = std::max({size.x, size.y, size.z});
    const float scale = extent > 0.0f ? 1.0f / extent : 1.0f;
    const Vec3 center = (lo + hi) * 0.5f;
    for (auto& v : m.v)
      v.pos = (v.pos - center) * scale;
  }
}

MeshData loadObj(const std::filesystem::path& path)
{
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("failed to open " + path.filename().string());

  std::vector<Vec3> positions, normals;
  std::vector<Vec2> texCoords;
  std::vector<std::array<Corner, 3>> triangles;

  std::string line;
  for (int lineNo = 1; std::getline(file, line); lineNo++)
  {
    try
    {
      std::istringstream ls(line);
      std::string tag;
      ls >> tag;

      if (tag == "v" || tag == "vn")
      {
        Vec3 p;
        if (!(ls >> p.x >> p.y >> p.z))
          throw std::runtime_error("malformed " + tag);
        (tag == "v" ? positions : normals).push_back(p);
      }
      else if (tag == "vt")
      {
        Vec2 uv;
        if (!(ls >> uv.x))
          throw std::runtime_error("malformed vt");
        ls >> uv.y;
        texCoords.push_back(uv);
      }
      else if (tag == "f")
      {
        std::vector<Corner> corners;
        std::string token;
        while (ls >> token)
          corners.push_back(parseCorner(token, positions.size(), texCoords.size(), normals.size()));

        for (size_t k = 1; k + 1 < corners.size(); k++)
          triangles.push_back({corners[0], corners[k], corners[k + 1]});
      }
    }
    catch (const std::exception& e)
    {
      throw std::runtime_error("line " + std::to_string(lineNo) + ": " + e.what());
    }
  }

  if (triangles.empty())
    throw std::runtime_error("no faces found in " + path.filename().string());

  // area-weighted smooth normals per position, used where the file gives none
  std::vector<Vec3> smooth(positions.size());
  for (const auto& t : triangles)
  {
    const Vec3 n = Vec3::cross(positions[t[1].v] - positions[t[0].v], positions[t[2].v] - positions[t[0].v]);
    for (const auto& c : t)
      smooth[c.v] += n;
  }

  MeshData m;
  std::map<std::array<int, 3>, uint32_t> lookup;
  for (const auto& t : triangles)
    for (const auto& c : t)
    {
      const auto [it, inserted] = lookup.try_emplace({c.v, c.t, c.n}, static_cast<uint32_t>(m.v.size()));
      if (inserted)
      {
        const Vec3 normal = Vec3::normalize(c.n >= 0 ? normals[c.n] : smooth[c.v]);
        m.v.push_back({positions[c.v], normal, c.t >= 0 ? texCoords[c.t] : Vec2{}});
      }
      m.i.push_back(it->second);
    }

  fitToUnitCube(m);
  return m;
}

#include "Shapes.h"

#include <algorithm>
#include <cmath>

using mdEngine::Vec2;
using mdEngine::Vec3;
using mdEngine::Vertex;

namespace
{
  constexpr float PI = 3.14159265f;

  Vec3 cross(const Vec3& a, const Vec3& b)
  {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
  }

  float dot(const Vec3& a, const Vec3& b)
  {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }

  Vec3 normalize(const Vec3& a)
  {
    return a / std::sqrt(dot(a, a));
  }

  uint32_t vertexCount(const MeshData& m)
  {
    return static_cast<uint32_t>(m.v.size());
  }

  void addTri(MeshData& m, const uint32_t a, const uint32_t b, const uint32_t c)
  {
    m.i.insert(m.i.end(), {a, b, c});
  }

  // n points on an ellipse, counter-clockwise, starting at the top
  std::vector<Vec2> ring(const int n, const float rx, const float ry)
  {
    std::vector<Vec2> pts;
    for (int k = 0; k < n; k++)
    {
      const float a = PI / 2 + 2 * PI * k / n;
      pts.push_back({std::cos(a) * rx, std::sin(a) * ry});
    }
    return pts;
  }

  // convex or star-shaped polygon in the XY plane, fanned around its average point
  void addPolygon(MeshData& m, const std::vector<Vec2>& pts)
  {
    Vec2 center{};
    for (const auto& p : pts) center += p;
    center /= static_cast<float>(pts.size());

    const uint32_t base = vertexCount(m);
    const uint32_t n = static_cast<uint32_t>(pts.size());
    m.v.push_back({{center.x, center.y, 0}, {0, 0, 1}, {center.x + 0.5f, center.y + 0.5f}});
    for (const auto& p : pts)
      m.v.push_back({{p.x, p.y, 0}, {0, 0, 1}, {p.x + 0.5f, p.y + 0.5f}});
    for (uint32_t k = 0; k < n; k++)
      addTri(m, base, base + 1 + k, base + 1 + (k + 1) % n);
  }

  // flat convex face of a solid centered at the origin, winding fixed so it faces outward
  void addFace(MeshData& m, std::vector<Vec3> pts)
  {
    Vec3 n = normalize(cross(pts[1] - pts[0], pts[2] - pts[0]));
    Vec3 sum{};
    for (const auto& p : pts) sum += p;
    if (dot(n, sum) < 0)
    {
      std::reverse(pts.begin(), pts.end());
      n = n * -1.0f;
    }

    // planar uvs in the face's own plane, scaled uniformly into [0, 1]
    const Vec3 t = normalize(pts[1] - pts[0]);
    const Vec3 b = cross(n, t);
    std::vector<Vec2> uvs;
    for (const auto& p : pts) uvs.push_back({dot(p - pts[0], t), dot(p - pts[0], b)});

    Vec2 lo = uvs[0], hi = uvs[0];
    for (const auto& uv : uvs)
    {
      lo = {std::min(lo.x, uv.x), std::min(lo.y, uv.y)};
      hi = {std::max(hi.x, uv.x), std::max(hi.y, uv.y)};
    }
    const float extent = std::max(hi.x - lo.x, hi.y - lo.y);

    const uint32_t base = vertexCount(m);
    for (size_t k = 0; k < pts.size(); k++)
      m.v.push_back({pts[k], n, {(uvs[k].x - lo.x) / extent, (uvs[k].y - lo.y) / extent}});
    for (uint32_t k = 1; k + 1 < pts.size(); k++)
      addTri(m, base, base + k, base + k + 1);
  }

  // (rows + 1) x (cols + 1) vertices from fn(u, v) in [0, 1], wound so that (col direction x row direction) is outward
  template <class F>
  void addGrid(MeshData& m, const int rows, const int cols, F fn)
  {
    const uint32_t base = vertexCount(m);
    for (int r = 0; r <= rows; r++)
      for (int c = 0; c <= cols; c++)
        m.v.push_back(fn(static_cast<float>(r) / rows, static_cast<float>(c) / cols));

    const uint32_t stride = cols + 1;
    for (int r = 0; r < rows; r++)
      for (int c = 0; c < cols; c++)
      {
        const uint32_t a = base + r * stride + c;
        addTri(m, a, a + 1, a + stride);
        addTri(m, a + stride, a + 1, a + stride + 1);
      }
  }

  void addCap(MeshData& m, const float y, const float radius, const bool up, const int slices)
  {
    const Vec3 n{0, up ? 1.0f : -1.0f, 0};
    const uint32_t base = vertexCount(m);
    m.v.push_back({{0, y, 0}, n, {0.5f, 0.5f}});
    for (int k = 0; k <= slices; k++)
    {
      const float a = 2 * PI * k / slices;
      m.v.push_back({{radius * std::cos(a), y, radius * std::sin(a)}, n, {0.5f + radius * std::cos(a), 0.5f + radius * std::sin(a)}});
    }
    for (int k = 0; k < slices; k++)
    {
      const uint32_t r0 = base + 1 + k;
      if (up) addTri(m, base, r0 + 1, r0);
      else addTri(m, base, r0, r0 + 1);
    }
  }

  MeshData makePolygon(const std::vector<Vec2>& pts)
  {
    MeshData m;
    addPolygon(m, pts);
    return m;
  }

  MeshData makeArrow()
  {
    MeshData m;
    addPolygon(m, {{-0.5f, -0.15f}, {0.1f, -0.15f}, {0.1f, 0.15f}, {-0.5f, 0.15f}});
    addPolygon(m, {{0.1f, -0.4f}, {0.5f, 0.0f}, {0.1f, 0.4f}});
    return m;
  }

  MeshData makeStar()
  {
    std::vector<Vec2> pts;
    for (int k = 0; k < 10; k++)
    {
      const float r = k % 2 == 0 ? 0.5f : 0.2f;
      const float a = PI / 2 + PI * k / 5;
      pts.push_back({std::cos(a) * r, std::sin(a) * r});
    }
    return makePolygon(pts);
  }

  MeshData makeCube()
  {
    MeshData m;
    const float uv[4][2] = {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}};
    for (int axis = 0; axis < 3; axis++)
      for (const float s : {-0.5f, 0.5f})
      {
        std::vector<Vec3> face;
        for (const auto& c : uv)
        {
          float p[3];
          p[axis] = s;
          p[(axis + 1) % 3] = c[0];
          p[(axis + 2) % 3] = c[1];
          face.push_back({p[0], p[1], p[2]});
        }
        addFace(m, face);
      }
    return m;
  }

  MeshData makeSphere()
  {
    MeshData m;
    addGrid(m, 24, 32, [](const float u, const float v)
    {
      const float theta = u * PI, phi = v * 2 * PI;
      const Vec3 n{std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi)};
      return Vertex{n * 0.5f, n, {v, 1.0f - u}};
    });
    return m;
  }

  // bottom radius rb, top radius rt, height 1 along Y
  MeshData makeFrustum(const float rb, const float rt, const bool capBottom, const bool capTop)
  {
    constexpr int slices = 32;
    MeshData m;
    addGrid(m, 1, slices, [&](const float u, const float v)
    {
      const float a = v * 2 * PI;
      const float r = rt + (rb - rt) * u;
      const Vec3 n = normalize({std::cos(a), rb - rt, std::sin(a)});
      return Vertex{{r * std::cos(a), 0.5f - u, r * std::sin(a)}, n, {v, 1.0f - u}};
    });
    if (capTop) addCap(m, 0.5f, rt, true, slices);
    if (capBottom) addCap(m, -0.5f, rb, false, slices);
    return m;
  }

  MeshData makeTetrahedron()
  {
    constexpr float R = 0.5f;
    const float y = -R / 3, r = R * 2 * std::sqrt(2.0f) / 3;
    Vec3 p[4] = {{0, R, 0}};
    for (int k = 0; k < 3; k++)
    {
      const float a = 2 * PI * k / 3;
      p[k + 1] = {r * std::cos(a), y, r * std::sin(a)};
    }

    MeshData m;
    addFace(m, {p[0], p[1], p[2]});
    addFace(m, {p[0], p[2], p[3]});
    addFace(m, {p[0], p[3], p[1]});
    addFace(m, {p[1], p[2], p[3]});
    return m;
  }

  MeshData makeTorus()
  {
    constexpr float R = 0.35f, r = 0.15f;
    MeshData m;
    addGrid(m, 32, 16, [](const float u, const float v)
    {
      const float t = u * 2 * PI, p = v * 2 * PI;
      const Vec3 n{std::cos(p) * std::cos(t), std::sin(p), std::cos(p) * std::sin(t)};
      const Vec3 c{R * std::cos(t), 0, R * std::sin(t)};
      return Vertex{c + n * r, n, {u, v}};
    });
    return m;
  }

  MeshData makePrism(const int sides)
  {
    std::vector<Vec3> bottom, top;
    for (int k = 0; k < sides; k++)
    {
      const float a = PI / 2 + 2 * PI * k / sides;
      const float x = std::cos(a) * 0.5f, z = std::sin(a) * 0.5f;
      bottom.push_back({x, -0.5f, z});
      top.push_back({x, 0.5f, z});
    }

    MeshData m;
    addFace(m, bottom);
    addFace(m, top);
    for (int k = 0; k < sides; k++)
    {
      const int j = (k + 1) % sides;
      addFace(m, {bottom[k], bottom[j], top[j], top[k]});
    }
    return m;
  }
} // namespace

MeshData makeShape(const ShapeType type)
{
  switch (type)
  {
    case ShapeType::Triangle: return makePolygon(ring(3, 0.5f, 0.5f));
    case ShapeType::Rectangle: return makePolygon({{-0.5f, -0.3f}, {0.5f, -0.3f}, {0.5f, 0.3f}, {-0.5f, 0.3f}});
    case ShapeType::Pentagon: return makePolygon(ring(5, 0.5f, 0.5f));
    case ShapeType::Hexagon: return makePolygon(ring(6, 0.5f, 0.5f));
    case ShapeType::Circle: return makePolygon(ring(64, 0.5f, 0.5f));
    case ShapeType::Ellipse: return makePolygon(ring(64, 0.5f, 0.3f));
    case ShapeType::Trapezoid: return makePolygon({{-0.5f, -0.3f}, {0.5f, -0.3f}, {0.25f, 0.3f}, {-0.25f, 0.3f}});
    case ShapeType::Star: return makeStar();
    case ShapeType::Arrow: return makeArrow();
    case ShapeType::Cube: return makeCube();
    case ShapeType::Sphere: return makeSphere();
    case ShapeType::Cylinder: return makeFrustum(0.5f, 0.5f, true, true);
    case ShapeType::Cone: return makeFrustum(0.5f, 0.0f, true, false);
    case ShapeType::TruncatedCone: return makeFrustum(0.5f, 0.25f, true, true);
    case ShapeType::Tetrahedron: return makeTetrahedron();
    case ShapeType::Torus: return makeTorus();
    case ShapeType::Prism: return makePrism(3);
    default: return {};
  }
}

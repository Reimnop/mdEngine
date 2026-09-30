#pragma once

#include <cstdint>
#include <vector>

#include "core/data/Vertex.h"

enum class ShapeType
{
  Triangle, Rectangle, Pentagon, Hexagon, Circle, Ellipse, Trapezoid, Star, Arrow,
  Cube, Sphere, Cylinder, Cone, TruncatedCone, Tetrahedron, Torus, Prism,
  Count
};

constexpr const char* SHAPE_NAMES[static_cast<int>(ShapeType::Count)] = {
  "Triangle", "Rectangle", "Pentagon", "Hexagon", "Circle", "Ellipse", "Trapezoid", "Star", "Arrow",
  "Cube", "Sphere", "Cylinder", "Cone", "Truncated Cone", "Tetrahedron", "Torus", "Prism"
};

struct MeshData
{
  std::vector<mdEngine::Vertex> v;
  std::vector<uint32_t> i;
};

inline bool is3D(const ShapeType t)
{
  return t >= ShapeType::Cube;
}

// 2D shapes lie in the XY plane facing +Z, 3D shapes are centered at the origin. All fit in [-0.5, 0.5].
MeshData makeShape(ShapeType type);

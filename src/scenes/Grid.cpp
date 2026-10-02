#include "Grid.h"

Grid::Grid(mdEngine::Renderer* rendererPtr): rendererPtr(rendererPtr)
{
  // generate grid mesh
  std::vector<mdEngine::Vertex> vertices;
  std::vector<uint32_t> indices;

  // rows
  for (auto i = -10; i <= 10; i++)
  {
    const mdEngine::Color3f color = i == 0
      ? mdEngine::Color3f(0.02f, 0.02f, 0.8f)
      : mdEngine::Color3f(0.02f, 0.02f, 0.02f);

    vertices.push_back({{static_cast<float>(i), 0.0f, -10.0f}, {0, 1, 0}, {0, 0}, color});
    vertices.push_back({{static_cast<float>(i), 0.0f, 10.0f}, {0, 1, 0}, {0, 1}, color});
    indices.push_back(vertices.size() - 2);
    indices.push_back(vertices.size() - 1);
  }

  // columns
  for (auto i = -10; i <= 10; i++)
  {
    const mdEngine::Color3f color = i == 0
      ? mdEngine::Color3f(0.8f, 0.02f, 0.02f)
      : mdEngine::Color3f(0.02f, 0.02f, 0.02f);

    vertices.push_back({{-10.0f, 0.0f, static_cast<float>(i)}, {0, 1, 0}, {0, 0}, color});
    vertices.push_back({{10.0f, 0.0f, static_cast<float>(i)}, {0, 1, 0}, {1, 0}, color});
    indices.push_back(vertices.size() - 2);
    indices.push_back(vertices.size() - 1);
  }

  this->gridMesh = rendererPtr->createMesh(
    vertices.data(), static_cast<GLsizei>(vertices.size()),
    indices.data(), static_cast<GLsizei>(indices.size()));
}

Grid::~Grid()
{
  this->rendererPtr->deleteMesh(this->gridMesh);
}

void Grid::render(std::vector<mdEngine::DrawObj>& drawObjs) const
{
  drawObjs.push_back(mdEngine::DrawObj{
    .meshHandle = this->gridMesh,
    .primitiveType = mdEngine::PrimitiveType::Lines,
    .shadingMode = mdEngine::ShadingMode::Flat
  });
}

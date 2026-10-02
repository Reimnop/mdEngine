#pragma once

#include "core/rendering/Renderer.h"

class Grid
{
public:
  explicit Grid(mdEngine::Renderer* rendererPtr);
  ~Grid();

  void render(std::vector<mdEngine::DrawObj>& drawObjs) const;
private:
  mdEngine::Renderer* rendererPtr;
  mdEngine::MeshHandle gridMesh;
};

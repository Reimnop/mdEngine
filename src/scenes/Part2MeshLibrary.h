#pragma once

#include "core/rendering/MeshHandle.h"
#include "core/rendering/Renderer.h"

class Part2MeshLibrary
{
public:
  mdEngine::MeshHandle sphere;
  mdEngine::MeshHandle cylinder;

  explicit Part2MeshLibrary(mdEngine::Renderer* rendererPtr);
  ~Part2MeshLibrary();
private:
  mdEngine::Renderer* rendererPtr;
};

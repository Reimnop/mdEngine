#pragma once

#include "core/Scene.h"
#include "core/rendering/Renderer.h"

class Part1 : public mdEngine::Scene
{
public:
  explicit Part1(mdEngine::Renderer* rendererPtr);
  ~Part1() override;
  void update() override;
  void render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj) override;
private:
  mdEngine::Renderer* rendererPtr;

  mdEngine::MeshHandle mesh;
};

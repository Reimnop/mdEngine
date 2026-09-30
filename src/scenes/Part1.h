#pragma once

#include "core/Scene.h"
#include "core/TrackballCamera.h"
#include "core/Window.h"
#include "core/rendering/Renderer.h"

class Part1 : public mdEngine::Scene
{
public:
  Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr);
  ~Part1() override;
  void update() override;
  void render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj) override;
private:
  mdEngine::Renderer* rendererPtr;
  mdEngine::TrackballCamera camera;

  mdEngine::MeshHandle mesh;
};

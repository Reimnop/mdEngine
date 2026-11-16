#pragma once

#include <vector>

#include "SceneBase.h"
#include "core/Window.h"
#include "core/rendering/Renderer.h"

class Part1 : public SceneBase
{
public:
  Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr);
  ~Part1() override;

  void update(float deltaTime) override;
  void renderGui() override;
protected:
  void render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj, mdEngine::PostProcessingObj& postProcessingObj) override;
};

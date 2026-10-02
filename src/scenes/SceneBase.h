#pragma once

#include <vector>

#include "Grid.h"
#include "core/Scene.h"
#include "core/OrbitCamera.h"
#include "core/Window.h"
#include "core/data/CameraObj.h"
#include "core/data/DrawObj.h"
#include "core/data/LightingObj.h"
#include "core/rendering/Renderer.h"

constexpr const char* FILL_MODE_NAMES[2] = {"Solid", "Wireframe"};
constexpr const char* SHADING_MODE_NAMES[4] = {"Inherit", "Flat", "Gouraud", "Phong"};

class SceneBase : public mdEngine::Scene
{
public:
  SceneBase(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr);

  void renderGui() override;
protected:
  mdEngine::Renderer* rendererPtr;
  mdEngine::OrbitCamera camera;

  virtual void render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj, mdEngine::PostProcessingObj& postProcessingObj);
private:
  std::vector<mdEngine::DrawObj> drawObjs;
  mdEngine::CameraObj cameraObj;
  mdEngine::LightingObj lightingObj;
  mdEngine::PostProcessingObj postProcessingObj;
  Grid grid;

  bool gridEnabled = false;
};

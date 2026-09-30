#pragma once

#include <vector>

#include "core/Scene.h"
#include "core/TrackballCamera.h"
#include "core/data/CameraObj.h"
#include "core/data/DrawObj.h"
#include "core/data/LightingObj.h"
#include "core/rendering/Renderer.h"

constexpr const char* FILL_MODE_NAMES[static_cast<int>(mdEngine::FillMode::Count)] = {"Solid", "Wireframe"};
constexpr const char* SHADING_MODE_NAMES[static_cast<int>(mdEngine::ShadingMode::Count)] = {"Flat", "Gouraud", "Phong"};

class SceneBase : public mdEngine::Scene
{
public:
  SceneBase(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr);

  void renderGui() override;
protected:
  mdEngine::Renderer* rendererPtr;
  mdEngine::TrackballCamera camera;

  virtual void render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj) = 0;
private:
  std::vector<mdEngine::DrawObj> drawObjs;
  mdEngine::CameraObj cameraObj;
  mdEngine::LightingObj lightingObj;
};

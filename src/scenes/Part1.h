#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "Shapes.h"
#include "core/Scene.h"
#include "core/TrackballCamera.h"
#include "core/Window.h"
#include "core/rendering/Renderer.h"

struct Entity
{
  std::string name;
  ShapeType shape = ShapeType::Cube;
  mdEngine::Vec3 position{};
  mdEngine::Vec3 rotation{}; // degrees
  mdEngine::Vec3 scale{1.0f, 1.0f, 1.0f};
  mdEngine::Vec3 color{0.8f, 0.8f, 0.8f};
};

constexpr const char* FILL_MODE_NAMES[static_cast<int>(mdEngine::FillMode::Count)] = {"Solid", "Wireframe"};
constexpr const char* SHADING_MODE_NAMES[static_cast<int>(mdEngine::ShadingMode::Count)] = {"Flat", "Gouraud", "Phong"};

class Part1 : public mdEngine::Scene
{
public:
  Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr);
  ~Part1() override;
  void update() override;
  void render() override;
  void renderGui() override;
private:
  Entity& addEntity(ShapeType shape);

  mdEngine::Renderer* rendererPtr;
  mdEngine::TrackballCamera camera;

  std::vector<mdEngine::DrawObj> drawObjs;
  mdEngine::CameraObj cameraObj;
  mdEngine::LightingObj lightingObj;

  std::array<mdEngine::MeshHandle, static_cast<int>(ShapeType::Count)> meshes;
  std::vector<Entity> entities;
  ShapeType selectedNewShape = ShapeType::Triangle;
  std::optional<size_t> selectedEntityIdx;

  size_t newEntityCounter = 0;
};

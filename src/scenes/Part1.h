#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Shapes.h"
#include "core/Scene.h"
#include "core/TrackballCamera.h"
#include "core/Window.h"
#include "core/rendering/Renderer.h"

struct MeshAsset
{
  std::string name;
  mdEngine::MeshHandle handle;
};

struct TextureAsset
{
  std::string name;
  mdEngine::TextureHandle handle;
  int width, height;
};

struct Entity
{
  std::string name;
  size_t mesh = 0; // index into Part1::meshes
  mdEngine::Vec3 position{};
  mdEngine::Vec3 rotation{}; // degrees
  mdEngine::Vec3 scale{1.0f, 1.0f, 1.0f};
  mdEngine::Vec3 color{0.8f, 0.8f, 0.8f};
  float shininess = 32.0f;
  std::optional<size_t> texture; // index into Part1::textures
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
  Entity& addEntity(size_t mesh);
  void importObj(const std::string& path);
  void loadTexture(const std::string& path);

  mdEngine::Renderer* rendererPtr;
  mdEngine::TrackballCamera camera;

  std::vector<mdEngine::DrawObj> drawObjs;
  mdEngine::CameraObj cameraObj;
  mdEngine::LightingObj lightingObj;

  std::vector<MeshAsset> meshes; // built-in shapes first (same order as ShapeType), then imported OBJs
  std::vector<TextureAsset> textures;
  std::vector<Entity> entities;
  size_t selectedNewMesh = 0;
  std::optional<size_t> selectedEntityIdx;

  size_t newEntityCounter = 0;

  char objPath[512] = "";
  std::string importStatus;

  char texturePath[512] = "";
  std::string textureStatus;
};

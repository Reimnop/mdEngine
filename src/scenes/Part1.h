#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ModelLoader.h"
#include "SceneBase.h"
#include "Transform.h"
#include "core/Window.h"
#include "core/rendering/Renderer.h"

struct SceneNode
{
  uint32_t id = 0;
  std::string name;
  mdEngine::Transform transform;
  bool visible = true; // hides the whole subtree
  std::vector<size_t> meshes; // indices into Model::meshes
  SceneNode* parent = nullptr;
  std::vector<std::unique_ptr<SceneNode>> children;
};

struct MeshAsset
{
  std::string name;
  mdEngine::MeshHandle handle;
  size_t material = 0; // index into Model::materials
  size_t vertexCount = 0;
  size_t triangleCount = 0;
};

struct MaterialAsset
{
  std::string name;
  mdEngine::Color3f color{0.8f, 0.8f, 0.8f};
  float shininess = 32.0f;
  float specularStrength = 0.5f;
  mdEngine::ShadingMode shadingMode = mdEngine::ShadingMode::Inherit;
  std::optional<size_t> texture; // index into Model::textures
};

struct TextureAsset
{
  mdEngine::TextureHandle handle;
  int width = 0, height = 0;
};

// an imported file: GPU resources plus the node tree that instances them
struct Model
{
  std::string name;
  std::vector<MeshAsset> meshes;
  std::vector<MaterialAsset> materials;
  std::vector<TextureAsset> textures;
  std::unique_ptr<SceneNode> root;
};

class Part1 : public SceneBase
{
public:
  Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr);
  ~Part1() override;

  void update(float deltaTime) override;
  void renderGui() override;
protected:
  void render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj, mdEngine::PostProcessingObj& postProcessingObj) override;
private:
  void importModel(const std::string& path);
  std::unique_ptr<SceneNode> buildNode(const ModelNode& source, SceneNode* parent);
  void destroyModel(Model& model) const;
  void removeModel(const Model* model);
  void removeNode(SceneNode* node);
  void clearScene();

  void drawImportPanel();
  void drawSceneTree();
  void drawTreeNode(Model& model, SceneNode& node);
  void drawInspector();
  static void drawMaterialEditor(MaterialAsset& material, const Model& model);
  static void collectDrawObjs(const Model& model, const SceneNode& node, const mdEngine::Mat4& parentWorld, std::vector<mdEngine::DrawObj>& drawObjs);

  std::vector<std::unique_ptr<Model>> models;
  SceneNode* selectedNode = nullptr;
  Model* selectedModel = nullptr;
  uint32_t nextNodeId = 1;

  char modelPath[512] = "";
  std::string importStatus;
  bool importFailed = false;
  std::vector<std::string> importWarnings;
};

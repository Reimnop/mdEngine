#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "ImageLoader.h"
#include "Transform.h"
#include "core/data/Vertex.h"
#include "core/math/Colors.h"
#include "core/math/Vectors.h"

struct ModelMesh
{
  std::string name;
  std::vector<mdEngine::Vertex> vertices;
  std::vector<uint32_t> indices;
  size_t material = 0; // index into ModelData::materials
  mdEngine::Vec3 boundsMin{}, boundsMax{};
};

struct ModelMaterial
{
  std::string name;
  mdEngine::Color3f color{0.8f, 0.8f, 0.8f};
  float shininess = 32.0f;
  float specularStrength = 0.5f;
  std::optional<size_t> texture; // index into ModelData::textures
};

struct ModelNode
{
  std::string name;
  mdEngine::Transform transform;
  std::vector<size_t> meshes; // indices into ModelData::meshes
  std::vector<ModelNode> children;
};

struct ModelData
{
  std::string name;
  std::vector<ModelMesh> meshes;
  std::vector<ModelMaterial> materials;
  std::vector<ImageData> textures;
  ModelNode root; // wrapper named after the file, its only child is the imported root node
  std::vector<std::string> warnings;
};

// throws std::runtime_error when the file can't be read or has no triangle geometry
ModelData loadModel(const std::filesystem::path& path);

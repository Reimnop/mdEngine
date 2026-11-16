#include "ModelLoader.h"

#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>

namespace
{
  using namespace mdEngine;

  std::string utf8(const std::filesystem::path& path)
  {
    const auto s = path.u8string();
    return {s.begin(), s.end()};
  }

  std::filesystem::path pathFromUtf8(const std::string& s)
  {
    return std::filesystem::path(std::u8string(s.begin(), s.end()));
  }

  Transform toTransform(const aiMatrix4x4& matrix)
  {
    aiVector3D scaling, position;
    aiQuaternion rotation;
    matrix.Decompose(scaling, rotation, position);

    // R = Rz * Ry * Rx, matching Transform::toMatrix
    const aiMatrix3x3 r = rotation.GetMatrix();
    const float y = std::asin(std::clamp(-r.c1, -1.0f, 1.0f));
    float x, z;
    if (std::abs(r.c1) < 0.99999f)
    {
      x = std::atan2(r.c2, r.c3);
      z = std::atan2(r.b1, r.a1);
    }
    else // gimbal lock, fold everything into x
    {
      x = std::atan2(-r.b3, r.b2);
      z = 0.0f;
    }

    return Transform{
      .position = {position.x, position.y, position.z},
      .rotation = {x * RAD_TO_DEG, y * RAD_TO_DEG, z * RAD_TO_DEG},
      .scale = {scaling.x, scaling.y, scaling.z}
    };
  }

  ModelMesh convertMesh(const aiMesh* mesh, const size_t index)
  {
    ModelMesh out;
    out.name = mesh->mName.length > 0 ? mesh->mName.C_Str() : "Mesh " + std::to_string(index);
    out.material = mesh->mMaterialIndex;

    const bool hasNormals = mesh->HasNormals();
    const bool hasUvs = mesh->HasTextureCoords(0);
    const bool hasColors = mesh->HasVertexColors(0);

    out.vertices.reserve(mesh->mNumVertices);
    Vec3 bmin{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    Vec3 bmax{-bmin.x, -bmin.y, -bmin.z};
    for (unsigned i = 0; i < mesh->mNumVertices; i++)
    {
      const auto& p = mesh->mVertices[i];
      Vertex v;
      v.pos = {p.x, p.y, p.z};
      if (hasNormals)
        v.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
      if (hasUvs)
        v.texCoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
      if (hasColors)
        v.color = Color3f(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b);
      out.vertices.push_back(v);

      bmin = {std::min(bmin.x, p.x), std::min(bmin.y, p.y), std::min(bmin.z, p.z)};
      bmax = {std::max(bmax.x, p.x), std::max(bmax.y, p.y), std::max(bmax.z, p.z)};
    }
    out.boundsMin = bmin;
    out.boundsMax = bmax;

    out.indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);
    for (unsigned i = 0; i < mesh->mNumFaces; i++)
    {
      const auto& face = mesh->mFaces[i];
      if (face.mNumIndices != 3)
        continue;
      out.indices.insert(out.indices.end(), face.mIndices, face.mIndices + 3);
    }
    return out;
  }

  ImageData decodeEmbeddedTexture(const aiTexture* texture)
  {
    if (texture->mHeight == 0) // compressed (png, jpg...), mWidth is the byte size
      return loadImageFromMemory(reinterpret_cast<const uint8_t*>(texture->pcData), texture->mWidth);

    ImageData image;
    image.width = static_cast<int>(texture->mWidth);
    image.height = static_cast<int>(texture->mHeight);
    image.pixels.resize(static_cast<size_t>(image.width) * image.height * 4);
    for (int y = 0; y < image.height; y++) // our images start at the bottom row
    {
      for (int x = 0; x < image.width; x++)
      {
        const auto& texel = texture->pcData[static_cast<size_t>(image.height - 1 - y) * image.width + x];
        auto* dst = &image.pixels[(static_cast<size_t>(y) * image.width + x) * 4];
        dst[0] = texel.r;
        dst[1] = texel.g;
        dst[2] = texel.b;
        dst[3] = texel.a;
      }
    }
    return image;
  }

  class TextureCollector
  {
  public:
    TextureCollector(const aiScene* scene, std::filesystem::path baseDir, ModelData& model):
      scene(scene), baseDir(std::move(baseDir)), model(model) {}

    std::optional<size_t> get(const aiMaterial* material)
    {
      aiString texPath;
      if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texPath) != AI_SUCCESS
        && material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) != AI_SUCCESS)
        return std::nullopt;

      const std::string key = texPath.C_Str();
      if (const auto it = cache.find(key); it != cache.end())
        return it->second;

      std::optional<size_t> result;
      try
      {
        ImageData image;
        if (const aiTexture* embedded = scene->GetEmbeddedTexture(key.c_str()))
        {
          image = decodeEmbeddedTexture(embedded);
        }
        else
        {
          auto rel = key;
          std::replace(rel.begin(), rel.end(), '\\', '/');
          auto path = baseDir / pathFromUtf8(rel);
          if (!std::filesystem::exists(path)) // exporters love absolute paths from someone else's machine
            path = baseDir / path.filename();
          image = loadImage(path);
        }
        model.textures.push_back(std::move(image));
        result = model.textures.size() - 1;
      }
      catch (const std::exception& e)
      {
        model.warnings.push_back("texture '" + key + "': " + e.what());
      }
      cache[key] = result;
      return result;
    }
  private:
    const aiScene* scene;
    std::filesystem::path baseDir;
    ModelData& model;
    std::map<std::string, std::optional<size_t>> cache;
  };

  ModelMaterial convertMaterial(const aiMaterial* material, const size_t index, TextureCollector& textures)
  {
    ModelMaterial out;
    aiString name;
    out.name = material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS && name.length > 0
      ? name.C_Str()
      : "Material " + std::to_string(index);

    aiColor4D color;
    if (material->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS
      || material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
      out.color = Color3f(color.r, color.g, color.b);

    float shininess = 0.0f;
    if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
      out.shininess = std::clamp(shininess, 1.0f, 256.0f);

    aiColor4D specular;
    if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS)
      out.specularStrength = std::clamp((specular.r + specular.g + specular.b) / 3.0f, 0.0f, 1.0f);

    out.texture = textures.get(material);
    if (out.texture.has_value()) // the texture supplies the color, a tint would just darken it
      out.color = Color3f(1.0f);
    return out;
  }

  ModelNode convertNode(const aiNode* node)
  {
    ModelNode out;
    out.name = node->mName.length > 0 ? node->mName.C_Str() : "Node";
    out.transform = toTransform(node->mTransformation);
    out.meshes.assign(node->mMeshes, node->mMeshes + node->mNumMeshes);
    out.children.reserve(node->mNumChildren);
    for (unsigned i = 0; i < node->mNumChildren; i++)
      out.children.push_back(convertNode(node->mChildren[i]));
    return out;
  }

  void expandBounds(const ModelData& model, const ModelNode& node, const Mat4& parentWorld, Vec3& bmin, Vec3& bmax, bool& any)
  {
    const Mat4 world = parentWorld * node.transform.toMatrix();
    for (const auto meshIdx : node.meshes)
    {
      const auto& mesh = model.meshes[meshIdx];
      if (mesh.indices.empty())
        continue;
      for (auto corner = 0; corner < 8; corner++)
      {
        const Vec4 local{
          (corner & 1) ? mesh.boundsMax.x : mesh.boundsMin.x,
          (corner & 2) ? mesh.boundsMax.y : mesh.boundsMin.y,
          (corner & 4) ? mesh.boundsMax.z : mesh.boundsMin.z,
          1.0f};
        const Vec4 p = world * local;
        if (!any)
        {
          bmin = bmax = {p.x, p.y, p.z};
          any = true;
          continue;
        }
        bmin = {std::min(bmin.x, p.x), std::min(bmin.y, p.y), std::min(bmin.z, p.z)};
        bmax = {std::max(bmax.x, p.x), std::max(bmax.y, p.y), std::max(bmax.z, p.z)};
      }
    }
    for (const auto& child : node.children)
      expandBounds(model, child, world, bmin, bmax, any);
  }
}

ModelData loadModel(const std::filesystem::path& path)
{
  if (!std::filesystem::is_regular_file(path))
    throw std::runtime_error("file not found: " + utf8(path.filename()));

  Assimp::Importer importer;
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

  const aiScene* scene = importer.ReadFile(utf8(path), 0
    | aiProcess_Triangulate
    | aiProcess_SortByPType
    | aiProcess_JoinIdenticalVertices
    | aiProcess_GenSmoothNormals
    | aiProcess_GenUVCoords
    | aiProcess_FindInvalidData
    | aiProcess_ValidateDataStructure);
  if (!scene || !scene->mRootNode)
    throw std::runtime_error(importer.GetErrorString());

  ModelData model;
  model.name = utf8(path.stem());

  TextureCollector textures(scene, path.parent_path(), model);
  for (unsigned i = 0; i < scene->mNumMaterials; i++)
    model.materials.push_back(convertMaterial(scene->mMaterials[i], i, textures));
  if (model.materials.empty())
    model.materials.push_back(ModelMaterial{.name = "Default"});

  size_t triangleCount = 0;
  for (unsigned i = 0; i < scene->mNumMeshes; i++)
  {
    auto mesh = convertMesh(scene->mMeshes[i], i);
    if (mesh.material >= model.materials.size())
      mesh.material = 0;
    triangleCount += mesh.indices.size() / 3;
    model.meshes.push_back(std::move(mesh));
  }
  if (triangleCount == 0)
    throw std::runtime_error("no triangle geometry found in " + utf8(path.filename()));

  model.root.name = model.name;
  model.root.children.push_back(convertNode(scene->mRootNode));
  return model;
}
#pragma once

#include <vector>
#include <glad/gl.h>

#include "MeshHandle.h"
#include "TextureHandle.h"
#include "core/data/CameraObj.h"
#include "core/data/DrawObj.h"
#include "core/data/LightingObj.h"
#include "core/data/PostProcessingObj.h"
#include "core/data/Vertex.h"
#include "core/memory/PooledSuballocator.h"
#include "post_processing/Aces.h"
#include "post_processing/Composite.h"

namespace mdEngine
{
  class Renderer
  {
  public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    GLuint renderFrame(
      const DrawObj* drawObjs, size_t drawObjCount,
      const CameraObj& cameraObj,
      const LightingObj& lightingObj,
      const PostProcessingObj& postProcessingObj,
      int width, int height);

    MeshHandle createMesh(const Vertex* vertices, GLsizei vertexCount, const uint32_t* indices, GLsizei indexCount);
    void deleteMesh(MeshHandle meshHandle);

    TextureHandle createTexture(GLsizei width, GLsizei height, const void* data);
    void deleteTexture(TextureHandle textureHandle);
  private:
    GLuint program = 0;
    GLint uMvpLocation = 0;
    GLint uModelLocation = 0;
    GLint uViewLocation = 0;
    GLint uShadingModeLocation = 0;
    GLint uColorLocation = 0;
    GLint uShininessLocation = 0;
    GLint uSpecularStrengthLocation = 0;
    GLint uLightDirLocation = 0;
    GLint uLightColorLocation = 0;
    GLint uAmbientLocation = 0;
    GLint uUseTextureLocation = 0;

    GLuint fbo = 0, postProcessingResolveFbo = 0;
    GLuint colorRboMsaa = 0, depthRbo = 0, postProcessingTex1 = 0, postProcessingTex2 = 0;
    int fboWidth = 1280, fboHeight = 720;

    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei vboSize = 1024;
    GLsizei eboSize = 1024;

    std::vector<Vertex> vertices = std::vector<Vertex>(vboSize);
    std::vector<uint32_t> indices = std::vector<uint32_t>(eboSize);
    PooledSuballocator vertexBufferAllocator{};
    PooledSuballocator indexBufferAllocator{};

    // post processors
    Composite compositePostProcessor{};
    Aces acesPostProcessor{};

    bool meshesDirty = true;

    void handleMeshUpdates();
    void handleFboResize(int width, int height);

    static void setVertexAttributesForVertex();
  };
} // mdEngine

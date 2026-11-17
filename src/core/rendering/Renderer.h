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
#include "post_processing/Bloom.h"
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

    TextureHandle createTexture(GLsizei width, GLsizei height, const void* data, bool isSrgb = false);
    void deleteTexture(TextureHandle textureHandle);
  private:
    GLuint mainProgram = 0;
    GLint mainMvpLocation = 0;
    GLint mainModelLocation = 0;
    GLint mainViewLocation = 0;
    GLint mainShadingModeLocation = 0;
    GLint mainColorLocation = 0;
    GLint mainShininessLocation = 0;
    GLint mainSpecularStrengthLocation = 0;
    GLint mainLightDirLocation = 0;
    GLint mainLightColorLocation = 0;
    GLint mainAmbientLocation = 0;
    GLint mainUseTextureLocation = 0;

    GLuint solidColorProgram = 0;
    GLint solidColorMvpLocation = 0;
    GLint solidColorColorLocation = 0;

    GLuint fbo = 0;
    GLuint colorRboMsaa = 0, depthRbo = 0;

    GLuint solidColorFbo = 0;
    GLuint solidColorTexture = 0, solidColorDepthTexture = 0;

    GLuint postProcessingResolveFbo = 0;
    GLuint postProcessingTex1 = 0, postProcessingTex2 = 0;

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
    Bloom bloomPostProcessor{};

    bool meshesDirty = true;

    void handleMeshUpdates();
    void handleFboResize(int width, int height);

    static void setVertexAttributesForVertex();
  };
} // mdEngine

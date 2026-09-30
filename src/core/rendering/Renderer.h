#pragma once

#include <vector>
#include <glad/gl.h>

#include "MeshHandle.h"
#include "core/Window.h"
#include "core/data/CameraObj.h"
#include "core/data/DrawObj.h"
#include "core/data/LightingObj.h"
#include "core/data/Vertex.h"

namespace mdEngine
{
  class Renderer
  {
  public:
    Renderer();
    ~Renderer();

    GLuint renderFrame(const DrawObj* drawObjs, size_t drawObjCount, const CameraObj& cameraObj, const LightingObj& lightingObj, int width, int height);

    MeshHandle addMesh(Vertex* vertices, GLsizei vertexCount, uint32_t* indices, GLsizei indexCount);
    void clearMeshes();
  private:
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei vboSize = 1024;
    GLsizei eboSize = 1024;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    bool meshesDirty = true;

    GLuint program = 0;
    GLint uMvpLocation = 0;
    GLint uModelLocation = 0;
    GLint uShadingModeLocation = 0;
    GLint uColorLocation = 0;
    GLint uLightDirLocation = 0;
    GLint uLightColorLocation = 0;
    GLint uAmbientLocation = 0;

    GLuint fbo;
    GLuint colorTex, depthRbo;
    int fboWidth = 1280, fboHeight = 720;

    void handleMeshUpdates();
    void handleFboResize(int width, int height);

    static void setVertexAttributesForVertex();
  };
} // mdEngine

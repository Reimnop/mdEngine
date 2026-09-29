#pragma once

#include <vector>
#include <glad/gl.h>

#include "MeshHandle.h"
#include "core/Window.h"
#include "core/data/CameraObj.h"
#include "core/data/DrawObj.h"
#include "core/data/Vertex.h"

namespace mdEngine
{
  class Renderer
  {
  public:
    Renderer(Window* windowPtr);
    ~Renderer();

    void renderFrame(const DrawObj* drawObjs, size_t drawObjCount, const CameraObj& cameraObj);

    MeshHandle addMesh(Vertex* vertices, GLsizei vertexCount, uint32_t* indices, GLsizei indexCount);
    void clearMeshes();
  private:
    Window* windowPtr;

    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei vboSize = 1024;
    GLsizei eboSize = 1024;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    bool meshesDirty = true;

    GLuint program = 0;
    GLint uMvpLocation = 0;

    static void setVertexAttributesForVertex();

    void handleMeshUpdates();
  };
} // mdEngine

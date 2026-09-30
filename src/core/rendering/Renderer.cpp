#include "Renderer.h"

#include "core/data/DrawObj.h"
#include "core/math/Vectors.h"
#include "helper/io.h"

namespace mdEngine
{
  Renderer::Renderer()
  {
    // init initial mesh
    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, vboSize * static_cast<GLsizei>(sizeof(Vertex)), nullptr, GL_DYNAMIC_DRAW);

    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    setVertexAttributesForVertex();

    glGenBuffers(1, &this->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboSize * static_cast<GLsizei>(sizeof(uint32_t)), nullptr, GL_DYNAMIC_DRAW);

    // setup fbo
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    // init shader
    const auto vss = io::readFile("assets/shaders/obj.vsh");
    const auto fss = io::readFile("assets/shaders/obj.fsh");

    const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const auto vssPtr = vss.c_str();
    glShaderSource(vertexShader, 1, &vssPtr, nullptr);
    glCompileShader(vertexShader);

    const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const auto fssPtr = fss.c_str();
    glShaderSource(fragmentShader, 1, &fssPtr, nullptr);
    glCompileShader(fragmentShader);

    this->program = glCreateProgram();
    glAttachShader(this->program, vertexShader);
    glAttachShader(this->program, fragmentShader);
    glLinkProgram(this->program);

    this->uMvpLocation = glGetUniformLocation(this->program, "uMvp");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  Renderer::~Renderer()
  {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &colorTex);
    glDeleteRenderbuffers(1, &depthRbo);
    glDeleteProgram(this->program);
  }

  GLuint Renderer::renderFrame(const DrawObj* drawObjs, const size_t drawObjCount, const CameraObj& cameraObj, const int width, const int height)
  {
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    auto proj = Mat4::perspective(cameraObj.fov, aspect, 0.1f, 1000.0f);

    handleMeshUpdates();
    handleFboResize(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(this->program);
    glBindVertexArray(this->vao);

    for (size_t i = 0; i < drawObjCount; i++)
    {
      const auto& drawObj = drawObjs[i];
      const auto& mesh = drawObj.meshHandle;

      auto mvp = proj * cameraObj.view * drawObj.transform;
      glUniformMatrix4fv(this->uMvpLocation, 1, GL_TRUE, mvp.m);
      glDrawElementsBaseVertex(
        GL_TRIANGLES,
        mesh.indexCount,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void*>(mesh.baseIndex * sizeof(uint32_t)),
        mesh.baseVertex);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return this->colorTex;
  }

  void Renderer::handleMeshUpdates()
  {
    if (!this->meshesDirty)
      return;
    this->meshesDirty = false;

    if (vertices.size() > this->vboSize)
    {
      this->vboSize = static_cast<GLsizei>(vertices.size());

      // delete and create new buffer
      glDeleteBuffers(1, &this->vbo);
      glGenBuffers(1, &this->vbo);
      glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
      glBufferData(GL_ARRAY_BUFFER, vboSize * static_cast<GLsizei>(sizeof(Vertex)), vertices.data(), GL_DYNAMIC_DRAW);

      // rebind to VAO
      glBindVertexArray(this->vao);
      setVertexAttributesForVertex();
    }
    else
    {
      // update existing buffer
      glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizei>(vertices.size() * sizeof(Vertex)), vertices.data());
    }

    glBindVertexArray(this->vao);

    if (indices.size() > this->eboSize)
    {
      this->eboSize = static_cast<GLsizei>(indices.size());

      // delete and create new buffer
      glDeleteBuffers(1, &this->ebo);
      glGenBuffers(1, &this->ebo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboSize * static_cast<GLsizei>(sizeof(uint32_t)), indices.data(), GL_DYNAMIC_DRAW);
    }
    else
    {
      // update existing buffer
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, static_cast<GLsizei>(indices.size() * sizeof(uint32_t)), indices.data());
    }
  }

  void Renderer::handleFboResize(int width, int height)
  {
    if (width == this->fboWidth && height == this->fboHeight)
      return;

    this->fboWidth = width;
    this->fboHeight = height;

    // delete, create new
    glDeleteTextures(1, &colorTex);
    glDeleteRenderbuffers(1, &depthRbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);
  }

  MeshHandle Renderer::addMesh(Vertex* vertices, const GLsizei vertexCount, uint32_t* indices, const GLsizei indexCount)
  {
    this->meshesDirty = true;

    // add vertices to vertex buffer
    this->vertices.insert(this->vertices.end(), vertices, vertices + vertexCount);

    // add indices to index buffer
    this->indices.insert(this->indices.end(), indices, indices + indexCount);

    return MeshHandle{
      .baseVertex = static_cast<GLint>(this->vertices.size() - vertexCount),
      .baseIndex = static_cast<GLint>(this->indices.size() - indexCount),
      .indexCount = indexCount
    };
  }

  void Renderer::clearMeshes()
  {
    this->vertices.clear();
    this->indices.clear();
  }

  void Renderer::setVertexAttributesForVertex()
  {
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(sizeof(Vec3)));

    // texCoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(2 * sizeof(Vec3)));
  }
} // mdEngine

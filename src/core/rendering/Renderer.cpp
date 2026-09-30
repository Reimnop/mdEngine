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
    const auto vss = io::readFileAsString("assets/shaders/obj.vsh");
    const auto fss = io::readFileAsString("assets/shaders/obj.fsh");

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
    this->uModelLocation = glGetUniformLocation(this->program, "uModel");
    this->uViewLocation = glGetUniformLocation(this->program, "uView");
    this->uShadingModeLocation = glGetUniformLocation(this->program, "uShadingMode");
    this->uColorLocation = glGetUniformLocation(this->program, "uColor");
    this->uShininessLocation = glGetUniformLocation(this->program, "uShininess");
    this->uSpecularStrengthLocation = glGetUniformLocation(this->program, "uSpecularStrength");
    this->uLightDirLocation = glGetUniformLocation(this->program, "uLightDir");
    this->uLightColorLocation = glGetUniformLocation(this->program, "uLightColor");
    this->uAmbientLocation = glGetUniformLocation(this->program, "uAmbient");
    this->uUseTextureLocation = glGetUniformLocation(this->program, "uUseTexture");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  Renderer::~Renderer()
  {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &colorTex);
    glDeleteRenderbuffers(1, &depthRbo);
    glDeleteProgram(this->program);
  }

  GLuint Renderer::renderFrame(const DrawObj* drawObjs, const size_t drawObjCount, const CameraObj& cameraObj, const LightingObj& lightingObj, const int width, const int height)
  {
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    auto proj = Mat4::perspective(cameraObj.fov, aspect, 0.1f, 1000.0f);

    handleMeshUpdates();
    handleFboResize(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);

    if (lightingObj.fillMode == FillMode::Wireframe)
      glDisable(GL_CULL_FACE);
    else
    {
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
    }

    glPolygonMode(GL_FRONT_AND_BACK, lightingObj.fillMode == FillMode::Wireframe ? GL_LINE : GL_FILL);

    glUseProgram(this->program);
    glBindVertexArray(this->vao);

    glUniform3f(this->uLightDirLocation, lightingObj.direction.x, lightingObj.direction.y, lightingObj.direction.z);
    glUniform3f(this->uLightColorLocation, lightingObj.color.x, lightingObj.color.y, lightingObj.color.z);
    glUniform1f(this->uAmbientLocation, lightingObj.ambient);
    glUniform1i(this->uShadingModeLocation, static_cast<GLint>(lightingObj.shadingMode));
    glUniform1f(this->uSpecularStrengthLocation, lightingObj.specularStrength);

    for (size_t i = 0; i < drawObjCount; i++)
    {
      const auto& drawObj = drawObjs[i];
      const auto& mesh = drawObj.meshHandle;

      auto mvp = proj * cameraObj.view * drawObj.transform;
      glUniformMatrix4fv(this->uMvpLocation, 1, GL_TRUE, mvp.m);
      glUniformMatrix4fv(this->uViewLocation, 1, GL_TRUE, cameraObj.view.m);
      glUniformMatrix4fv(this->uModelLocation, 1, GL_TRUE, drawObj.transform.m);
      glUniform3f(this->uColorLocation, drawObj.color.x, drawObj.color.y, drawObj.color.z);
      glUniform1f(this->uShininessLocation, drawObj.shininess);
      glUniform1i(this->uUseTextureLocation, drawObj.texture.has_value() ? 1 : 0);
      glBindTexture(GL_TEXTURE_2D, drawObj.texture.has_value() ? drawObj.texture.value().handle : 0);
      glDrawElementsBaseVertex(
        GL_TRIANGLES,
        mesh.indexCount,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void*>(mesh.indexAllocation.offset * sizeof(uint32_t)),
        static_cast<GLint>(mesh.vertexAllocation.offset));
    }

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

  MeshHandle Renderer::createMesh(const Vertex* vertices, const GLsizei vertexCount, const uint32_t* indices, const GLsizei indexCount)
  {
    this->meshesDirty = true;

    // allocate space on the vertex buffer
    const auto vertexAllocation = vertexBufferAllocator.allocate(vertexCount, [&](const size_t newCapacity)
    {
      this->vertices.resize(newCapacity);
    });

    // copy vertices into the allocated space
    std::copy(vertices, vertices + vertexCount, this->vertices.begin() + vertexAllocation.offset);

    // allocate space on the index buffer
    const auto indexAllocation = indexBufferAllocator.allocate(indexCount, [&](const size_t newCapacity)
    {
      this->indices.resize(newCapacity);
    });

    // copy indices into the allocated space
    std::copy(indices, indices + indexCount, this->indices.begin() + indexAllocation.offset);

    return MeshHandle{
      .vertexAllocation = vertexAllocation,
      .indexAllocation = indexAllocation,
      .indexCount = indexCount
    };
  }

  void Renderer::deleteMesh(MeshHandle meshHandle)
  {
    vertexBufferAllocator.free(meshHandle.vertexAllocation);
    indexBufferAllocator.free(meshHandle.indexAllocation);
  }

  TextureHandle Renderer::createTexture(GLsizei width, GLsizei height, const void* data)
  {
    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return TextureHandle{.handle = texHandle};
  }

  void Renderer::deleteTexture(TextureHandle textureHandle)
  {
    glDeleteTextures(1, &textureHandle.handle);
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

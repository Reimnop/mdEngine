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

    glGenRenderbuffers(1, &colorRboMsaa);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRboMsaa);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA16F, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRboMsaa);

    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    // setup resolve fbo
    glGenFramebuffers(1, &postProcessingResolveFbo);

    // setup post-processing textures
    glGenTextures(1, &postProcessingTex1);
    glBindTexture(GL_TEXTURE_2D, postProcessingTex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fboWidth, fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenTextures(1, &postProcessingTex2);
    glBindTexture(GL_TEXTURE_2D, postProcessingTex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fboWidth, fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
    glDeleteRenderbuffers(1, &colorRboMsaa);
    glDeleteRenderbuffers(1, &depthRbo);
    glDeleteFramebuffers(1, &postProcessingResolveFbo);
    glDeleteTextures(1, &postProcessingTex1);
    glDeleteTextures(1, &postProcessingTex2);
    glDeleteProgram(this->program);
  }

  GLuint Renderer::renderFrame(
    const DrawObj* drawObjs, const size_t drawObjCount,
    const CameraObj& cameraObj,
    const LightingObj& lightingObj,
    const PostProcessingObj& postProcessingObj,
    const int width, const int height)
  {
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    auto proj = Mat4::perspective(cameraObj.fov, aspect, 0.1f, 1000.0f);

    handleMeshUpdates();
    handleFboResize(width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    const auto clearColorLinear = lightingObj.clearColor.toLinear();
    glClearColor(clearColorLinear.r, clearColorLinear.g, clearColorLinear.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);

    glEnable(GL_MULTISAMPLE);
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

    const auto lightColorLinear = lightingObj.color.toLinear() * lightingObj.intensity;
    const auto ambientLinear = ch::toLinear(lightingObj.ambient);

    glUniform3f(this->uLightColorLocation, lightColorLinear.r, lightColorLinear.g, lightColorLinear.b);
    glUniform1f(this->uAmbientLocation, ambientLinear);

    auto globalLightingMode = lightingObj.shadingMode == ShadingMode::Inherit ? ShadingMode::Phong : lightingObj.shadingMode;

    for (size_t i = 0; i < drawObjCount; i++)
    {
      const auto& drawObj = drawObjs[i];
      const auto& mesh = drawObj.meshHandle;

      const auto mvp = proj * cameraObj.view * drawObj.transform;
      const auto colorLinear = drawObj.color.toLinear();

      glUniformMatrix4fv(this->uMvpLocation, 1, GL_TRUE, mvp.m);
      glUniformMatrix4fv(this->uViewLocation, 1, GL_TRUE, cameraObj.view.m);
      glUniformMatrix4fv(this->uModelLocation, 1, GL_TRUE, drawObj.transform.m);
      glUniform1i(this->uShadingModeLocation,
        (drawObj.shadingMode == ShadingMode::Inherit ? static_cast<int>(globalLightingMode) : static_cast<int>(drawObj.shadingMode)) - 1);
      glUniform1f(this->uSpecularStrengthLocation, drawObj.specularStrength);
      glUniform3f(this->uColorLocation, colorLinear.r, colorLinear.g, colorLinear.b);
      glUniform1f(this->uShininessLocation, drawObj.shininess);
      glUniform1i(this->uUseTextureLocation, drawObj.texture.has_value() ? 1 : 0);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, drawObj.texture.has_value() ? drawObj.texture.value().handle : 0);

      glDrawElementsBaseVertex(
        drawObj.primitiveType == PrimitiveType::Lines ? GL_LINES : GL_TRIANGLES,
        mesh.indexCount,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void*>(mesh.indexAllocation.offset * sizeof(uint32_t)),
        static_cast<GLint>(mesh.vertexAllocation.offset));
    }

    // bind tex1 to resolve fbo
    glBindFramebuffer(GL_FRAMEBUFFER, postProcessingResolveFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessingTex1, 0);

    // resolve MSAA with resolve FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postProcessingResolveFbo);
    glBlitFramebuffer(
      0, 0,
      width, height,
      0, 0,
      width, height,
      GL_COLOR_BUFFER_BIT,
      GL_NEAREST);

    // do post-processing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_MULTISAMPLE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // bloom runs on linear HDR, so it goes before tonemapping
    if (postProcessingObj.useBloom && bloomPostProcessor.process(
      postProcessingTex1, postProcessingTex2, width, height,
      postProcessingObj.bloomIntensity, postProcessingObj.bloomDiffusion,
      postProcessingObj.bloomThreshold, postProcessingObj.bloomKnee, postProcessingObj.bloomColor))
      std::swap(postProcessingTex1, postProcessingTex2);

    if (postProcessingObj.useTonemapping && acesPostProcessor.process(postProcessingTex1, postProcessingTex2, width, height, postProcessingObj.exposure))
      std::swap(postProcessingTex1, postProcessingTex2);

    if (compositePostProcessor.process(postProcessingTex1, postProcessingTex2, width, height))
      std::swap(postProcessingTex1, postProcessingTex2);

    return this->postProcessingTex1;
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
    glDeleteRenderbuffers(1, &colorRboMsaa);
    glDeleteRenderbuffers(1, &depthRbo);
    glDeleteTextures(1, &postProcessingTex1);
    glDeleteTextures(1, &postProcessingTex2);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // re-init MSAA renderbuffers
    glGenRenderbuffers(1, &colorRboMsaa);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRboMsaa);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA16F, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRboMsaa);

    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    // re-init post-processing textures
    glGenTextures(1, &postProcessingTex1);
    glBindTexture(GL_TEXTURE_2D, postProcessingTex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fboWidth, fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenTextures(1, &postProcessingTex2);
    glBindTexture(GL_TEXTURE_2D, postProcessingTex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fboWidth, fboHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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

  TextureHandle Renderer::createTexture(const GLsizei width, const GLsizei height, const void* data)
  {
    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return TextureHandle{.handle = texHandle};
  }

  void Renderer::deleteTexture(const TextureHandle textureHandle)
  {
    glDeleteTextures(1, &textureHandle.handle);
  }

  void Renderer::setVertexAttributesForVertex()
  {
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, pos)));

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, normal)));

    // texCoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, texCoord)));

    // color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));
  }
} // mdEngine

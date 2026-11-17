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

    {
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
    }

    {
      // setup solid color fbo
      glGenFramebuffers(1, &solidColorFbo);
      glBindFramebuffer(GL_FRAMEBUFFER, solidColorFbo);

      glGenTextures(1, &solidColorTexture);
      glBindTexture(GL_TEXTURE_2D, solidColorTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, solidColorTexture, 0);

      glGenTextures(1, &solidColorDepthTexture);
      glBindTexture(GL_TEXTURE_2D, solidColorDepthTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, solidColorDepthTexture, 0);
    }

    {
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
    }

    // init main shader program
    {
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

      this->mainProgram = glCreateProgram();
      glAttachShader(this->mainProgram, vertexShader);
      glAttachShader(this->mainProgram, fragmentShader);
      glLinkProgram(this->mainProgram);

      this->mainMvpLocation = glGetUniformLocation(this->mainProgram, "uMvp");
      this->mainModelLocation = glGetUniformLocation(this->mainProgram, "uModel");
      this->mainViewLocation = glGetUniformLocation(this->mainProgram, "uView");
      this->mainShadingModeLocation = glGetUniformLocation(this->mainProgram, "uShadingMode");
      this->mainColorLocation = glGetUniformLocation(this->mainProgram, "uColor");
      this->mainShininessLocation = glGetUniformLocation(this->mainProgram, "uShininess");
      this->mainSpecularStrengthLocation = glGetUniformLocation(this->mainProgram, "uSpecularStrength");
      this->mainLightDirLocation = glGetUniformLocation(this->mainProgram, "uLightDir");
      this->mainLightColorLocation = glGetUniformLocation(this->mainProgram, "uLightColor");
      this->mainAmbientLocation = glGetUniformLocation(this->mainProgram, "uAmbient");
      this->mainUseTextureLocation = glGetUniformLocation(this->mainProgram, "uUseTexture");

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
    }

    // init solid color shader program
    {
      const auto vss = io::readFileAsString("assets/shaders/objsolidcolor.vsh");
      const auto fss = io::readFileAsString("assets/shaders/objsolidcolor.fsh");

      const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
      const auto vssPtr = vss.c_str();
      glShaderSource(vertexShader, 1, &vssPtr, nullptr);
      glCompileShader(vertexShader);

      const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      const auto fssPtr = fss.c_str();
      glShaderSource(fragmentShader, 1, &fssPtr, nullptr);
      glCompileShader(fragmentShader);

      this->solidColorProgram = glCreateProgram();
      glAttachShader(this->solidColorProgram, vertexShader);
      glAttachShader(this->solidColorProgram, fragmentShader);
      glLinkProgram(this->solidColorProgram);

      this->solidColorMvpLocation = glGetUniformLocation(this->solidColorProgram, "uMvp");
      this->solidColorColorLocation = glGetUniformLocation(this->solidColorProgram, "uColor");

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
    }
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
    glDeleteProgram(this->mainProgram);

    glDeleteFramebuffers(1, &solidColorFbo);
    glDeleteTextures(1, &solidColorTexture);
    glDeleteTextures(1, &solidColorDepthTexture);
    glDeleteProgram(this->solidColorProgram);
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
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glUseProgram(this->mainProgram);
    glBindVertexArray(this->vao);

    glUniform3f(this->mainLightDirLocation, lightingObj.direction.x, lightingObj.direction.y, lightingObj.direction.z);

    const auto lightColorLinear = lightingObj.color.toLinear() * lightingObj.intensity;
    const auto ambientLinear = ch::toLinear(lightingObj.ambient);

    glUniform3f(this->mainLightColorLocation, lightColorLinear.r, lightColorLinear.g, lightColorLinear.b);
    glUniform1f(this->mainAmbientLocation, ambientLinear);

    for (size_t i = 0; i < drawObjCount; i++)
    {
      const auto& drawObj = drawObjs[i];
      const auto& mesh = drawObj.meshHandle;

      const auto mvp = proj * cameraObj.view * drawObj.transform;
      const auto colorLinear = drawObj.color.toLinear();

      glUniformMatrix4fv(this->mainMvpLocation, 1, GL_TRUE, mvp.m);
      glUniformMatrix4fv(this->mainViewLocation, 1, GL_TRUE, cameraObj.view.m);
      glUniformMatrix4fv(this->mainModelLocation, 1, GL_TRUE, drawObj.transform.m);
      glUniform1i(this->mainShadingModeLocation, static_cast<GLint>(drawObj.shadingMode));
      glUniform1f(this->mainSpecularStrengthLocation, drawObj.specularStrength);
      glUniform3f(this->mainColorLocation, colorLinear.r, colorLinear.g, colorLinear.b);
      glUniform1f(this->mainShininessLocation, drawObj.shininess);
      glUniform1i(this->mainUseTextureLocation, drawObj.texture.has_value() ? 1 : 0);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, drawObj.texture.has_value() ? drawObj.texture.value().handle : 0);

      glDrawElementsBaseVertex(
        drawObj.primitiveType == PrimitiveType::Lines ? GL_LINES : GL_TRIANGLES,
        mesh.indexCount,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void*>(mesh.indexAllocation.offset * sizeof(uint32_t)),
        static_cast<GLint>(mesh.vertexAllocation.offset));
    }

    glDisable(GL_MULTISAMPLE);

    glBindFramebuffer(GL_FRAMEBUFFER, solidColorFbo);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(this->solidColorProgram);

    for (size_t i = 0; i < drawObjCount; i++)
    {
      const auto& drawObj = drawObjs[i];
      if (!drawObj.drawInIdPass) {
        continue;
      }

      const auto& mesh = drawObj.meshHandle;

      const auto mvp = proj * cameraObj.view * drawObj.transform;

      glUniformMatrix4fv(this->solidColorMvpLocation, 1, GL_TRUE, mvp.m);
      glUniform3f(
        this->solidColorColorLocation,
        drawObj.idColor.r / 255.0f,
        drawObj.idColor.g / 255.0f,
        drawObj.idColor.b / 255.0f);

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
    glDeleteTextures(1, &solidColorTexture);
    glDeleteTextures(1, &solidColorDepthTexture);
    glDeleteTextures(1, &postProcessingTex1);
    glDeleteTextures(1, &postProcessingTex2);

    // re-init MSAA renderbuffers
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &colorRboMsaa);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRboMsaa);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA16F, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRboMsaa);

    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

    // re-init solid color textures
    glBindFramebuffer(GL_FRAMEBUFFER, solidColorFbo);

    glGenTextures(1, &solidColorTexture);
    glBindTexture(GL_TEXTURE_2D, solidColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, solidColorTexture, 0);

    glGenTextures(1, &solidColorDepthTexture);
    glBindTexture(GL_TEXTURE_2D, solidColorDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, solidColorDepthTexture, 0);

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

  TextureHandle Renderer::createTexture(const GLsizei width, const GLsizei height, const void* data, bool isSrgb)
  {
    const GLint internalFormat = isSrgb
      ? GL_SRGB8_ALPHA8
      : GL_RGBA8;

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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

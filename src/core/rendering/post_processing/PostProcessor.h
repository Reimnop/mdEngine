#pragma once

#include <glad/gl.h>

#include "helper/io.h"

namespace mdEngine
{
  class PostProcessor
  {
  public:
    PostProcessor();
    virtual ~PostProcessor();
  protected:
    GLuint vao = 0, fbo = 0;

    static GLuint createVertexShader();
    static GLuint createProgram(const char* fragmentShaderPath);
  };
} // mdEngine

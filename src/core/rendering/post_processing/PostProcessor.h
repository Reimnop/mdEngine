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

    virtual bool process(GLuint inputTexture, GLuint outputTexture, GLsizei width, GLsizei height) = 0;
  protected:
    GLuint vao = 0, fbo = 0;

    static GLuint createVertexShader();
  };
} // mdEngine

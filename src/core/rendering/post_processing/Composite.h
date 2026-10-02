#pragma once

#include "PostProcessor.h"

namespace mdEngine
{
  class Composite : public PostProcessor
  {
  public:
    Composite();
    ~Composite() override;

    bool process(GLuint inputTexture, GLuint outputTexture, GLsizei width, GLsizei height);
  private:
    GLuint program = 0;
  };
} // mdEngine

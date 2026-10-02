#pragma once

#include "PostProcessor.h"

namespace mdEngine
{
  class Composite : public PostProcessor
  {
  public:
    Composite();
    ~Composite() override;

    Composite(const Composite&) = delete;
    Composite& operator=(const Composite&) = delete;

    bool process(GLuint inputTexture, GLuint outputTexture, GLsizei width, GLsizei height);
  private:
    GLuint program = 0;
  };
} // mdEngine

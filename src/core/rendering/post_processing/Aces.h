#pragma once

#include "PostProcessor.h"

namespace mdEngine
{
  class Aces : public PostProcessor
  {
  public:
    Aces();
    ~Aces() override;

    bool process(GLuint inputTexture, GLuint outputTexture, GLsizei width, GLsizei height, float exposure);
  private:
    GLuint program = 0;
    GLint uExposureLocation = 0;
  };
} // mdEngine

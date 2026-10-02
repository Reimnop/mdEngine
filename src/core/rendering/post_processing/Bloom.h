#pragma once

#include <vector>

#include "PostProcessor.h"
#include "core/math/Colors.h"

namespace mdEngine
{
  class Bloom : public PostProcessor
  {
  public:
    Bloom();
    ~Bloom() override;

    Bloom(const Bloom&) = delete;
    Bloom& operator=(const Bloom&) = delete;

    bool process(
      GLuint inputTexture, GLuint outputTexture, GLsizei width, GLsizei height,
      float intensity, float diffusion, float threshold, float knee, const Color3f& color);
  private:
    struct Mip
    {
      GLuint handle;
      GLsizei width, height;
    };

    GLuint prefilterProgram = 0, downsampleProgram = 0, upsampleProgram = 0, combineProgram = 0;
    GLint uThresholdLocation = 0, uKneeLocation = 0, uSampleScaleLocation = 0, uTintLocation = 0;

    GLuint sampler = 0;

    std::vector<Mip> mipChain;
    GLsizei currentWidth = -1, currentHeight = -1;
    int currentIterations = 0;

    void updateMipChain(GLsizei width, GLsizei height, int levels);
    void clearMipChain();
  };
} // mdEngine

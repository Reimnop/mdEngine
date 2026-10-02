#include "Bloom.h"

#include <algorithm>
#include <cmath>

namespace mdEngine
{
  Bloom::Bloom()
  {
    prefilterProgram = createProgram("assets/shaders/post_processing/bloom/prefilter.fsh");
    downsampleProgram = createProgram("assets/shaders/post_processing/bloom/downsample.fsh");
    upsampleProgram = createProgram("assets/shaders/post_processing/bloom/upsample.fsh");
    combineProgram = createProgram("assets/shaders/post_processing/bloom/combine.fsh");

    uThresholdLocation = glGetUniformLocation(prefilterProgram, "uThreshold");
    uKneeLocation = glGetUniformLocation(prefilterProgram, "uKnee");
    uSampleScaleLocation = glGetUniformLocation(upsampleProgram, "uSampleScale");
    uTintLocation = glGetUniformLocation(combineProgram, "uTint");

    // combine samples the source on unit 0 and the bloom on unit 1
    glUseProgram(combineProgram);
    glUniform1i(glGetUniformLocation(combineProgram, "uSource"), 0);
    glUniform1i(glGetUniformLocation(combineProgram, "uBloom"), 1);

    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  Bloom::~Bloom()
  {
    glDeleteProgram(prefilterProgram);
    glDeleteProgram(downsampleProgram);
    glDeleteProgram(upsampleProgram);
    glDeleteProgram(combineProgram);
    glDeleteSamplers(1, &sampler);
    clearMipChain();
  }

  bool Bloom::process(
    const GLuint inputTexture, const GLuint outputTexture, const GLsizei width, const GLsizei height,
    float intensity, const float diffusion, const float threshold, const float knee, const Color3f& color)
  {
    if (intensity <= 0.0f || width <= 0 || height <= 0)
      return false;

    // determine iteration count and sample scale
    const auto s = static_cast<float>(std::max(width, height));
    const auto logS = std::log2(s) + std::min(diffusion, 10.0f) - 10.0f; // 10 is the base diffusion
    const auto logSFloored = std::floor(logS);
    const auto iterations = static_cast<int>(std::clamp(logSFloored, 1.0f, 16.0f));
    const auto sampleScale = 0.5f + (logS - logSFloored);

    // rebuild the mip chain if needed
    if (width != currentWidth || height != currentHeight || iterations != currentIterations)
    {
      currentWidth = width;
      currentHeight = height;
      currentIterations = iterations;
      updateMipChain(width, height, iterations);
    }

    if (mipChain.size() < 2)
      return false;

    const auto& mip0 = mipChain[0];

    glBindVertexArray(vao);
    glBindSampler(0, sampler);
    glBindSampler(1, sampler);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDisable(GL_BLEND);

    // prefilter: input -> mip 0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip0.handle, 0);
    glViewport(0, 0, mip0.width, mip0.height);

    glUseProgram(prefilterProgram);
    glUniform1f(uThresholdLocation, threshold);
    glUniform1f(uKneeLocation, threshold * knee);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // downsample: mip[i - 1] -> mip[i]
    glUseProgram(downsampleProgram);

    for (size_t i = 1; i < mipChain.size(); i++)
    {
      const auto& source = mipChain[i - 1];
      const auto& target = mipChain[i];

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.handle, 0);
      glViewport(0, 0, target.width, target.height);

      glBindTexture(GL_TEXTURE_2D, source.handle);
      glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    // upsample: mip[i + 1] -> mip[i], accumulated with additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glUseProgram(upsampleProgram);
    glUniform1f(uSampleScaleLocation, sampleScale);

    for (auto i = static_cast<int>(mipChain.size()) - 2; i >= 0; i--)
    {
      const auto& source = mipChain[i + 1];
      const auto& target = mipChain[i];

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.handle, 0);
      glViewport(0, 0, target.width, target.height);

      glBindTexture(GL_TEXTURE_2D, source.handle);
      glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glDisable(GL_BLEND);

    // combine: input + mip 0 * tint -> output
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);
    glViewport(0, 0, width, height);

    glUseProgram(combineProgram);

    // tint is the bloom color in linear space,
    // normalized to unit luminance,
    // scaled by intensity
    auto tint = color.toLinear();
    const auto luminance = 0.2126f * tint.r + 0.7152f * tint.g + 0.0722f * tint.b;
    tint = luminance > 0.0f ? tint * (1.0f / luminance) : Color3f(1.0f);
    tint *= intensity;
    glUniform3f(uTintLocation, tint.r, tint.g, tint.b);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mip0.handle);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // restore state
    glBindSampler(0, 0);
    glBindSampler(1, 0);
    glActiveTexture(GL_TEXTURE0);

    return true;
  }

  void Bloom::updateMipChain(const GLsizei width, const GLsizei height, const int levels)
  {
    clearMipChain();

    for (int i = 0; i < levels; i++)
    {
      const auto mipWidth = std::max(width >> i, 1);
      const auto mipHeight = std::max(height >> i, 1);

      GLuint handle;
      glGenTextures(1, &handle);
      glBindTexture(GL_TEXTURE_2D, handle);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mipWidth, mipHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
      // the sampler overrides these, but keep the texture complete without it
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      mipChain.push_back(Mip{handle, mipWidth, mipHeight});

      if (mipWidth == 1 && mipHeight == 1)
        break; // reached 1x1
    }
  }

  void Bloom::clearMipChain()
  {
    for (const auto& mip : mipChain)
      glDeleteTextures(1, &mip.handle);
    mipChain.clear();
  }
} // mdEngine

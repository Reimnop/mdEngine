#include "TrackballCamera.h"

#include <algorithm>
#include <cmath>
#include <imgui.h>

namespace mdEngine
{
  namespace
  {
    void mapToTrackball(const double px, const double py, const int w, const int h, float out[3])
    {
      const float s = static_cast<float>(std::min(w, h));
      const float x = (2.0f * static_cast<float>(px) - static_cast<float>(w)) / s;
      const float y = (static_cast<float>(h) - 2.0f * static_cast<float>(py)) / s;
      const float d2 = x * x + y * y;
      const float z = d2 <= 0.5f ? std::sqrt(1.0f - d2) : 0.5f / std::sqrt(d2);
      const float len = std::sqrt(d2 + z * z);
      out[0] = x / len;
      out[1] = y / len;
      out[2] = z / len;
    }

    void orthonormalize(Mat4& r)
    {
      float* m = r.m;
      const auto normalize = [](float* a)
      {
        const float l = std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
        a[0] /= l;
        a[1] /= l;
        a[2] /= l;
      };

      float a[3] = {m[0], m[1], m[2]};
      float b[3] = {m[4], m[5], m[6]};
      normalize(a);
      const float d = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
      for (int i = 0; i < 3; i++) b[i] -= d * a[i];
      normalize(b);
      const float c[3] = {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
      };
      for (int i = 0; i < 3; i++)
      {
        m[i] = a[i];
        m[4 + i] = b[i];
        m[8 + i] = c[i];
      }
    }
  } // namespace

  TrackballCamera::TrackballCamera(Window* windowPtr, const float distance)
    : windowPtr(windowPtr), initialDistance(distance), distance(distance)
  {
  }

  void TrackballCamera::update(ImGuiIO& io)
  {
    updateDrag(io);
    updateZoom(io);
  }

  void TrackballCamera::reset()
  {
    rotation = Mat4{};
    target = Vec3{};
    distance = initialDistance;
    dragMode = DragMode::None;
  }

  Mat4 TrackballCamera::getViewMatrix() const
  {
    return Mat4::translate(0.0f, 0.0f, -distance) * rotation * Mat4::translate(-target.x, -target.y, -target.z);
  }

  void TrackballCamera::setDistance(const float d)
  {
    distance = std::clamp(d, minDistance, maxDistance);
  }

  void TrackballCamera::setDistanceLimits(const float minD, const float maxD)
  {
    minDistance = minD;
    maxDistance = maxD;
    distance = std::clamp(distance, minDistance, maxDistance);
  }

  void TrackballCamera::updateDrag(ImGuiIO& io)
  {
    DragMode mode = DragMode::None;
    if (io.MouseDown[ImGuiMouseButton_Left])
    {
      mode = DragMode::Rotate;
    }
    else if (io.MouseDown[ImGuiMouseButton_Right] || io.MouseDown[ImGuiMouseButton_Middle])
    {
      mode = DragMode::Pan;
    }

    float x = io.MousePos.x, y = io.MousePos.y;

    // Starting (or switching) a drag only records the cursor position.
    const bool started = mode != dragMode;
    dragMode = mode;
    const double prevX = lastX, prevY = lastY;
    lastX = x;
    lastY = y;
    if (mode == DragMode::None || started) return;

    int w, h;
    windowPtr->getSize(w, h);
    if (w <= 0 || h <= 0) return;

    if (mode == DragMode::Rotate) rotateBy(prevX, prevY, w, h);
    else panBy(x - prevX, y - prevY, h);
  }

  void TrackballCamera::rotateBy(const double prevX, const double prevY, const int w, const int h)
  {
    float p0[3], p1[3];
    mapToTrackball(prevX, prevY, w, h, p0);
    mapToTrackball(lastX, lastY, w, h, p1);

    // Rotation axis (in view space) and angle between the two trackball points.
    const float axis[3] = {
      p0[1] * p1[2] - p0[2] * p1[1],
      p0[2] * p1[0] - p0[0] * p1[2],
      p0[0] * p1[1] - p0[1] * p1[0]
    };
    const float axisLen = std::sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
    if (axisLen < 1e-6f) return;

    const float dot = std::clamp(p0[0] * p1[0] + p0[1] * p1[1] + p0[2] * p1[2], -1.0f, 1.0f);
    const float angle = std::acos(dot);

    rotation = Mat4::rotate(angle, axis[0] / axisLen, axis[1] / axisLen, axis[2] / axisLen) * rotation;
    orthonormalize(rotation);
  }

  void TrackballCamera::panBy(const double dx, const double dy, const int h)
  {
    // World units covered by one pixel at the target's depth.
    const float unitsPerPixel = 2.0f * distance * std::tan(fov * 0.5f) / static_cast<float>(h);

    // Cursor delta in view space (screen right / screen up).
    const float vx = static_cast<float>(dx) * unitsPerPixel;
    const float vy = -static_cast<float>(dy) * unitsPerPixel;

    // Rotate into world space (multiply by the transpose of the rotation) and move the
    // target the opposite way so the scene follows the cursor.
    const float* m = rotation.m;
    target.x -= vx * m[0] + vy * m[4];
    target.y -= vx * m[1] + vy * m[5];
    target.z -= vx * m[2] + vy * m[6];
  }

  void TrackballCamera::updateZoom(ImGuiIO& io)
  {
    const float scroll = io.MouseWheel;
    if (scroll == 0.0f) return;

    // Exponential zoom feels uniform at any distance; scrolling up zooms in.
    setDistance(distance * std::exp(-scroll * zoomSpeed));
  }
} // mdEngine

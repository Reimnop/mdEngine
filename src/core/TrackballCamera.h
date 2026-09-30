#pragma once

#include <imgui.h>

#include "core/Window.h"
#include "core/data/CameraObj.h"
#include "core/math/Matrices.h"

namespace mdEngine
{
  // camera controlled by a virtual trackball
  // - left mouse : rotate
  // - right/middle mouse : pan
  // - scroll : zoom
  class TrackballCamera
  {
  public:
    explicit TrackballCamera(Window* windowPtr, float distance = 5.0f);

    void update(ImGuiIO& io);
    void reset();

    [[nodiscard]] Mat4 getViewMatrix() const;

    [[nodiscard]] float getDistance() const { return distance; }
    void setDistance(float d);
    void setDistanceLimits(float minDistance, float maxDistance);
    void setZoomSpeed(float speed) { zoomSpeed = speed; }
    void setFov(float fovRadians) { fov = fovRadians; }
  private:
    enum class DragMode { None, Rotate, Pan };

    void updateDrag(ImGuiIO& io);
    void rotateBy(double x, double y, int w, int h);
    void panBy(double x, double y, int h);
    void updateZoom(ImGuiIO& io);

    Window* windowPtr;

    Mat4 rotation{};
    Vec3 target{}; // world-space point the camera orbits around
    DragMode dragMode = DragMode::None;
    float lastX = 0.0f;
    float lastY = 0.0f;

    float fov = CameraObj{}.fov;

    float initialDistance;
    float distance;
    float minDistance = 1.0f;
    float maxDistance = 50.0f;
    float zoomSpeed = 0.1f; // fraction of the distance per scroll step
  };
} // mdEngine

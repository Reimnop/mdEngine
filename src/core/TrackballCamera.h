#pragma once

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

    void update();
    void reset();

    [[nodiscard]] Mat4 getViewMatrix() const;

    [[nodiscard]] float getDistance() const { return distance; }
    void setDistance(float d);
    void setDistanceLimits(float minDistance, float maxDistance);
    void setZoomSpeed(float speed) { zoomSpeed = speed; }
    void setFov(float fovRadians) { fov = fovRadians; }
  private:
    enum class DragMode { None, Rotate, Pan };

    void updateDrag();
    void rotateBy(double x, double y, int w, int h);
    void panBy(double x, double y, int h);
    void updateZoom();

    Window* windowPtr;

    Mat4 rotation{};
    Vec3 target{}; // world-space point the camera orbits around
    DragMode dragMode = DragMode::None;
    double lastX = 0.0;
    double lastY = 0.0;

    float fov = CameraObj{}.fov;

    float initialDistance;
    float distance;
    float minDistance = 1.0f;
    float maxDistance = 50.0f;
    float zoomSpeed = 0.1f; // fraction of the distance per scroll step
  };
} // mdEngine

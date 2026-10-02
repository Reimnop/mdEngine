#pragma once

#include <imgui.h>

#include "core/data/CameraObj.h"
#include "core/math/Matrices.h"

namespace mdEngine
{
  // Blender orbit camera
  class OrbitCamera
  {
  public:
    explicit OrbitCamera(float distance = 5.0f);

    // hovered: the cursor is over the viewport image, viewportHeight: its height in pixels
    void update(const ImGuiIO& io, bool hovered, float viewportHeight);
    void reset();

    [[nodiscard]] Mat4 getViewMatrix() const;
    [[nodiscard]] float getDistance() const { return distance; }
    void setFov(const float fovRadians) { fov = fovRadians; }
  private:
    enum class DragMode { None, Orbit, Pan, Zoom };

    void updateDrag(const ImGuiIO& io, bool hovered, float viewportHeight);
    void updateKeys(const ImGuiIO& io);
    void setDistance(float d);

    float yaw = 0.0f; // radians, rotation around the up axis
    float pitch = 0.0f; // radians, positive looks down on the scene
    Vec3 target{}; // world-space point the camera orbits around

    float initialDistance;
    float distance;
    float fov = CameraObj{}.fov;

    DragMode dragMode = DragMode::None;
    ImGuiMouseButton dragButton = ImGuiMouseButton_Middle;
  };
} // mdEngine
